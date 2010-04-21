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


#if GROV_DEVELOP_MODE || GROV_OFFLINE_DEVELOPMENT
	#include <QtCore/QFile>
#endif

#include <QtCore/QUrl>

#include <src/common.hpp>
#include <src/common/feed_item.hpp>
#include <src/main.hpp>

#include <src/client/storage.hpp>

#include "download_feeds_items.hpp"
#include "get_feed_list.hpp"
#include "gr_xml_parser.hpp"

#include "get_reading_list.hpp"


namespace grov { namespace client { namespace reader { namespace tasks {


Get_reading_list::Get_reading_list(Storage* storage, const QString& login, const QString& password, QObject* parent)
:
	Google_reader_task(login, password, parent),
	storage(storage),
	reading_lists_counter(0)
{
}



void Get_reading_list::authenticated(void)
{
	Get_feed_list* task = new Get_feed_list(this->storage, this->auth_id, this);

	connect(task, SIGNAL(feeds_gotten()),
		this, SLOT(get_reading_list()), Qt::QueuedConnection );

	this->process_task(task);
}



void Get_reading_list::get_reading_list(void)
{
	MLIB_D("Getting Google Reader's reading list...");

#if GROV_OFFLINE_DEVELOPMENT
	QFile list("reading.list");

	if(list.open(QIODevice::ReadOnly))
		this->request_finished("", list.readAll());
	else
		this->request_finished(_F("Error while reading '%1'.", list.fileName()), "");
#else
	QString url =
		"https://www.google.com/reader/atom/user/-/state/com.google/reading-list"
		"?n=1000&r=o&xt=user/-/state/com.google/read"
		"&client=" + QUrl::toPercentEncoding(get_user_agent());

	if(!this->continuation_code.isEmpty())
		url += "&c=" + this->continuation_code;

	this->get(url);
#endif
}



void Get_reading_list::on_items_downloaded(void)
{
	this->finish();
	emit this->reading_list_gotten();
}



void Get_reading_list::on_reading_list_gotten(void)
{
	#if 0 && GROV_DEVELOP_MODE
		this->on_items_downloaded();
	#else
		Download_feeds_items* task = new Download_feeds_items(this->storage, this);

		connect(task, SIGNAL(downloaded()),
			this, SLOT(on_items_downloaded()), Qt::QueuedConnection );

		this->process_task(task);
	#endif
}



void Get_reading_list::request_finished(const QString& error, const QByteArray& reply)
{
	MLIB_D("Reading list request finished.");

	try
	{
		Gr_feed_item_list items;

		try
		{
			// Checking for errors -->
				if(this->throw_if_fatal_error(error))
				{
					MLIB_D("Request failed. Trying again...");
					this->get_reading_list();
					return;
				}
			// Checking for errors <--

		#if GROV_DEVELOP_MODE && !GROV_OFFLINE_DEVELOPMENT
			// For offline development -->
			{
				QFile list("reading.list");
				list.open(QIODevice::WriteOnly);
				list.write(reply);
			}
			// For offline development <--
		#endif

			// Getting feeds' items -->
				try
				{
					items = Gr_xml_parser().reading_list(reply, &this->continuation_code);
				}
				catch(m::Exception& e)
				{
					M_THROW(PAM( tr("Parsing error."), EE(e) ));
				}
			// Getting feeds' items <--
		}
		catch(m::Exception& e)
		{
			M_THROW(PAM( tr("Unable to get Google Reader's reading list."), EE(e) ));
		}

		// Throws m::Exception
		this->storage->add_items(items);
		this->reading_lists_counter++;

	#if !GROV_OFFLINE_DEVELOPMENT
		if(this->continuation_code.isEmpty() || items.empty())
	#endif
			this->on_reading_list_gotten();
	#if !GROV_OFFLINE_DEVELOPMENT
		else
		{
			if(this->reading_lists_counter >= 100)
			{
				MLIB_W(tr("Reading lists number limit exceeded"),
					_F(tr(
						"Gotten too big number (%1) of reading lists from Google Reader. "
						"It seems that program entered in an infinite loop, or you have very big number of unread items. "
						"Please read downloaded items first, than you can download others."
					), this->reading_lists_counter)
				);
				this->on_reading_list_gotten();
			}
			else
				this->get_reading_list();
		}
	#endif
	}
	catch(m::Exception& e)
	{
		this->failed(EE(e));
	}
}


}}}}

