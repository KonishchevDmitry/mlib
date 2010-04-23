/**************************************************************************
*                                                                         *
*   grov - Google Reader offline viewer                                   *
*                                                                         *
*   Copyright (C) 2010, Dmitry Konishchev                                 *
*   http://konishchevdmitry.blogspot.com/                                 *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          *
*   GNU General Public License for more details.                          *
*                                                                         *
**************************************************************************/


#include <algorithm>

#include <QtCore/QUrl>

#include <grov/common.hpp>
#include <grov/main.hpp>

#include <grov/client/storage.hpp>

#include "flush_offline_data.hpp"


namespace grov { namespace client { namespace reader { namespace tasks {


namespace {


	/// Functor for comparing two Changed_feed_item's.
	struct Changed_feed_items_compare
	{
		bool operator()(const Changed_feed_item& a, const Changed_feed_item& b)
		{
			if(a.property != b.property)
				return a.property < b.property;
			else
				return a.value < b.value;
		}
	};


}


Flush_offline_data::Flush_offline_data(Storage* storage, const QString& login, const QString& password, QObject* parent)
:
	Google_reader_task(login, password, parent),
	storage(storage)
{
}



void Flush_offline_data::authenticated(void)
{
	this->get_token();
}



void Flush_offline_data::flush(void)
{
#if GROV_OFFLINE_DEVELOPMENT
	this->finish();
	emit this->flushed();
#else
	// If we flushed all data -->
		if(this->flush_start == this->changed_items.end())
		{
			this->finish();
			emit this->flushed();
			return;
		}
	// If we flushed all data <--

	// Flushing next item -->
	{
		QString items_string;
		Changed_feed_item::Changed_property change;
		bool changed_value;

		// Creating &i=...&i=...&i=... string
		// -->
		{
			size_t added_items = 0;
			Changed_feed_item_list::const_iterator it = this->flush_start;
			Changed_feed_item_list::const_iterator end = this->changed_items.end();

			while(it != end && added_items < 20)
			{
				const Changed_feed_item& item = *it;

				if(!added_items)
				{
					change = item.property;
					changed_value = item.value;
				}
				else
					if(item.property != change || item.value != changed_value)
						break;

				MLIB_D("Flushing item [%1](%2) changes (%3, %4)...", item.id, item.gr_id, change, changed_value);
				items_string += "&i=" + QUrl::toPercentEncoding(item.gr_id);
				++added_items;
				++it;
			}

			this->flush_end = it;
		}
		// <--

		QString url = "https://www.google.com/reader/api/0/";
		QString post_data;

		// Creating POST data -->
			switch(change)
			{
				case Changed_feed_item::PROPERTY_READ:
				#if GROV_DEVELOP_MODE
					MLIB_D("Skipping flushing for developer mode.");
					this->flush_start = this->flush_end;
					this->flush();
					return;
				#else
					url += "edit-tag";
					post_data = _F(
						"%1=%2&T=%3",
						changed_value ? 'a' : 'r',
						QUrl::toPercentEncoding("user/-/state/com.google/read"),
						QUrl::toPercentEncoding(this->token)
					);
				#endif
					break;

				case Changed_feed_item::PROPERTY_STARRED:
					url += "edit-tag";
					post_data = _F(
						"%1=%2&T=%3",
						changed_value ? 'a' : 'r',
						QUrl::toPercentEncoding("user/-/state/com.google/starred"),
						QUrl::toPercentEncoding(this->token)
					);
					break;

				default:
				#if GROV_DEVELOP_MODE
					MLIB_LE();
				#endif
					break;
			}
		// Creating POST data <--

		url += "?client=" + QUrl::toPercentEncoding(get_user_agent());
		this->post(url, post_data + items_string);
	}
	// Flushing next item <--
#endif
}



void Flush_offline_data::request_finished(const QString& error, const QByteArray& reply)
{
	MLIB_D("Flushing item's changes request finished.");

	try
	{
		try
		{
			// Checking for errors -->
			{
				QString general_error = error;
				QString server_answer = reply.trimmed();

				if(general_error.isEmpty() && server_answer != "OK")
				{
					if(server_answer.isEmpty())
						general_error = tr("Server returned error.");
					else
						general_error = server_answer;
				}

				if(this->throw_if_fatal_error(general_error))
				{
					MLIB_D("Request failed. Trying again...");
					this->flush();
					return;
				}
			}
			// Checking for errors <--
		}
		catch(m::Exception& e)
		{
			M_THROW(PAM( tr("Unable flush user changes to Google Reader."), EE(e) ));
		}

		// Synchronizing flushes with the database
		// Throws m::Exception
		this->storage->mark_changes_as_flushed(this->flush_start, this->flush_end);
		this->flush_start = this->flush_end;

		this->flush();
	}
	catch(m::Exception& e)
	{
		this->failed(EE(e));
	}
}



void Flush_offline_data::token_gotten(void)
{
	try
	{
		this->changed_items = this->storage->get_user_changes();
	}
	catch(m::Exception& e)
	{
		this->failed(EE(e));
		return;
	}

	/// Grouping changes, so we can flush them by bundles.
	std::sort(this->changed_items.begin(), this->changed_items.end(),
		Changed_feed_items_compare() );

	this->flush_start = this->changed_items.begin();
	this->flush_end = this->flush_start;

	MLIB_D("Flushing all offline data...");
	this->flush();
}


}}}}

