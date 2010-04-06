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

// TODO
#include <QtNetwork/QNetworkRequest>

#include <src/common.hpp>

#include <src/client/reader.hpp>
#include <src/client/storage.hpp>

#include "get_gr_token.hpp"

#include "flush_offline_data.hpp"


namespace grov { namespace client { namespace reader { namespace tasks {


Flush_offline_data::Flush_offline_data(Reader* reader, QObject* parent)
:
	Google_reader_task(reader, parent)
{
}



void Flush_offline_data::flush(void)
{
// TODO
#if !OFFLINE_DEVELOPMENT
	emit this->flushed();
#else
	// If we flushed all data -->
		if(changed_items_flush == changed_items_end)
		{
			try
			{
				this->sync_with_db();
			}
			catch(m::Exception& e)
			{
				emit this->error(EE(e));
				return;
			}

			emit this->flushed();
			return;
		}
	// If we flushed all data <--

	// Flushing next item -->
	{
		const Changed_feed_item& item = *changed_items_flush;
		// TODO: other params
		QString url = "https://www.google.com/reader/api/0/";
		QString post_request;

		MLIB_D("Flushing item [%1](%2) changes (%3, %4)...", item.id, item.gr_id, item.property, item.value);
		switch(item.property)
		{
			case Changed_feed_item::PROPERTY_READ:
				// TODO
				++this->changed_items_flush;
				this->flush();
				return;
				break;

			case Changed_feed_item::PROPERTY_STARRED:
				url += "edit-tag";
				post_request = _F(
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

		this->post(url, post_request.toAscii());
	}
	// Flushing next item <--
#endif
}



void Flush_offline_data::process(void)
{
	// TODO: delete on cancel and other actions...
	Get_gr_token* task = new Get_gr_token(this->reader, this);

	// TODO: this and all other
	connect(task, SIGNAL(error(const QString&)),
		this, SIGNAL(error(const QString&)) );

	connect(task, SIGNAL(token_gotten(const QString&)),
		this, SLOT(token_gotten(const QString&)) );

	task->process();
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

			++this->changed_items_flush;
		}
		catch(m::Exception& e)
		{
			try
			{
				this->sync_with_db();
			}
			catch(m::Exception& e)
			{
				MLIB_SW(EE(e));
			}

			M_THROW(PAM( tr("Unable flush user changes to Google Reader."), EE(e) ));
		}

		// Synchronizing flushes with the database
		if(this->changed_items_flush - this->changed_items_db >= 10)
			// Throws m::Exception
			this->sync_with_db();

		this->flush();
	}
	catch(m::Exception& e)
	{
		emit this->error(EE(e));
	}
}



void Flush_offline_data::sync_with_db(void)
{
	// Throws m::Exception
	this->reader->storage->mark_changes_as_flushed(this->changed_items_db, this->changed_items_flush);
	this->changed_items_db = this->changed_items_flush;
}



void Flush_offline_data::token_gotten(const QString& token)
{
	this->token = token;

	try
	{
		this->changed_items = this->reader->storage->get_user_changes();
	}
	catch(m::Exception& e)
	{
		emit this->error(EE(e));
		return;
	}

	this->changed_items_db = this->changed_items.begin();
	this->changed_items_flush = this->changed_items.begin();
	this->changed_items_end = this->changed_items.end();

	MLIB_D("Flushing all offline data...");
	this->flush();
}


}}}}

