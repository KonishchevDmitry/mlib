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


#include <QtCore/QUrl>

#include <src/common.hpp>
#include <src/main.hpp>

#include <src/client/storage.hpp>

#include "flush_offline_data.hpp"


namespace grov { namespace client { namespace reader { namespace tasks {


Flush_offline_data::Flush_offline_data(Storage* storage, const QString& login, const QString& password, QObject* parent)
:
	Google_reader_task(login, password, parent),
	storage(storage),
	to_db(this->changed_items.begin()),
	to_flush(this->changed_items.begin())
{
}



Flush_offline_data::~Flush_offline_data(void)
{
	if(this->is_cancelled())
		this->silent_sync_with_db();
}



void Flush_offline_data::authenticated(void)
{
	this->get_token();
}



void Flush_offline_data::flush(void)
{
#if OFFLINE_DEVELOPMENT
	emit this->flushed();
	this->finish();
#else
	// If we flushed all data -->
		if(this->to_flush == this->changed_items.end())
		{
			try
			{
				this->sync_with_db();
			}
			catch(m::Exception& e)
			{
				this->failed(EE(e));
				return;
			}

			emit this->flushed();
			this->finish();
			return;
		}
	// If we flushed all data <--

	// Flushing next item -->
	{
		const Changed_feed_item& item = *this->to_flush;
		QString url = "https://www.google.com/reader/api/0/";
		QString post_data;

		MLIB_D("Flushing item [%1](%2) changes (%3, %4)...", item.id, item.gr_id, item.property, item.value);
		switch(item.property)
		{
			case Changed_feed_item::PROPERTY_READ:
			#if DEVELOP_MODE
				MLIB_D("Skipping flushing for developer mode.");
				++this->to_flush;
				this->flush();
				return;
			#else
				url += "edit-tag";
				post_data = _F(
					"i=%1&%2=%3&T=%4",
					QUrl::toPercentEncoding(item.gr_id),
					item.value ? 'a' : 'r',
					QUrl::toPercentEncoding("user/-/state/com.google/read"),
					QUrl::toPercentEncoding(this->token)
				);
			#endif
				break;

			case Changed_feed_item::PROPERTY_STARRED:
				url += "edit-tag";
				post_data = _F(
					"i=%1&%2=%3&T=%4",
					QUrl::toPercentEncoding(item.gr_id),
					item.value ? 'a' : 'r',
					QUrl::toPercentEncoding("user/-/state/com.google/starred"),
					QUrl::toPercentEncoding(this->token)
				);
				break;

			default:
			#if DEVELOP_MODE
				MLIB_LE();
			#endif
				break;
		}

		url += "?client=" + QUrl::toPercentEncoding(get_user_agent());
		this->post(url, post_data);
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

			++this->to_flush;
		}
		catch(m::Exception& e)
		{
			this->silent_sync_with_db();
			M_THROW(PAM( tr("Unable flush user changes to Google Reader."), EE(e) ));
		}

		// Synchronizing flushes with the database
		if(this->to_flush - this->to_db >= 10)
			// Throws m::Exception
			this->sync_with_db();

		this->flush();
	}
	catch(m::Exception& e)
	{
		this->failed(EE(e));
	}
}



void Flush_offline_data::silent_sync_with_db(void)
{
	try
	{
		this->sync_with_db();
	}
	catch(m::Exception& e)
	{
		MLIB_SW(EE(e));
	}
}



void Flush_offline_data::sync_with_db(void)
{
	// Throws m::Exception
	this->storage->mark_changes_as_flushed(this->to_db, this->to_flush);
	this->to_db = this->to_flush;
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

	this->to_db = this->changed_items.begin();
	this->to_flush = this->changed_items.begin();

	MLIB_D("Flushing all offline data...");
	this->flush();
}


}}}}

