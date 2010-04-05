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


// TODO
#include <QtCore/QFile>

#include <QtNetwork/QNetworkRequest>

#include <src/common.hpp>
#include <src/common/feed_item.hpp>

#include <src/client/reader.hpp>
#include <src/client/storage.hpp>

#include "gr_xml_parser.hpp"

#include "get_reading_list.hpp"


namespace grov { namespace client { namespace reader { namespace tasks {


Get_reading_list::Get_reading_list(Reader* reader, QObject* parent)
:
	Google_reader_task(reader, parent)
{
}



void Get_reading_list::get_reading_list(void)
{
#if OFFLINE_DEVELOPMENT
	QFile reading_list("reading_list");
	reading_list.open(QIODevice::ReadOnly);
	QByteArray reply = reading_list.readAll();
	reading_list.close();

	this->request_finished("", reply);
#else
	MLIB_D("Getting Google Reader's reading list...");

	// TODO: other params
	// TODO: more xt
	QString query = "https://www.google.com/reader/atom/user/-/state/com.google/reading-list?n=1000&r=o&xt=user/-/state/com.google/read";

	if(!this->continuation_code.isEmpty())
		query += "&c=" + this->continuation_code;

// TODO:
//	this->post("https://www.google.com/reader/api/0/edit-tag?client=contact:konishchev",
//	//"a=user%2F14394675015157700687%2Fstate%2Fcom.google%2Fstarred&async=true&s=feed%2Fhttp%3A%2F%2Fwww.opennet.ru%2Fopennews%2Fopennews_mini.rss&i=tag%3Agoogle.com%2C2005%3Areader%2Fitem%2F0e114e09b80a285c&pos=0&T=6CoOqycBAAA.GONvLRCRL9agdExKsQJdhQ.n-UQldYDsiz58ao-SZDdpA"
//	"a=user%2F-%2Fstate%2Fcom.google%2Fstarred&async=true&s=feed%2Fhttp%3A%2F%2Fwww.opennet.ru%2Fopennews%2Fopennews_mini.rss&i=tag%3Agoogle.com%2C2005%3Areader%2Fitem%2F0e114e09b80a285c&T=9ysSqycBAAA.GONvLRCRL9agdExKsQJdhQ.ZuwUM5_HhNb9_D0H7Pxh9g"
//	//"i=" + QUrl::toPercentEncoding("tag:google.com,2005:reader/item/8e6564550a865a10") + "&a=" + QUrl::toPercentEncoding("user/14394675015157700687/state/com.google/starred")
//	);
	this->get(query);
#endif
}



void Get_reading_list::request_finished(const QString& error, const QByteArray& reply)
{
	MLIB_D("Reading list request finished.");

	try
	{
		Gr_feed_item_list items;

		// TODO
//		{
//			QFile reading_list("reading_list");
//			reading_list.open(QIODevice::Append);
//			reading_list.write(reply);
//			reading_list.close();
//		}

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

			// Getting feeds' items -->
				try
				{
					items = Gr_xml_parser().reading_list(reply, &this->continuation_code);
				}
				catch(m::Exception& e)
				{
					M_THROW(PAM( tr("Parsing error:"), EE(e) ));
				}
			// Getting feeds' items <--
		}
		catch(m::Exception& e)
		{
			M_THROW(tr("Unable to get Google Reader's reading list. %1"), EE(e));
		}

		// Throws m::Exception
		this->reader->storage->add_items(items);

#if !OFFLINE_DEVELOPMENT
		// TODO
		if(this->continuation_code.isEmpty() || items.empty())
#endif
			emit this->reading_list_gotten();
#if !OFFLINE_DEVELOPMENT
		else
			// TODO max number limit
			this->get_reading_list();
#endif
	}
	catch(m::Exception& e)
	{
		emit this->error(EE(e));
	}
}



void Get_reading_list::process(void)
{
	// TODO: delete on cancel and other actions...
	this->get_feed_list_task = new Get_feed_list(this->reader, this);

	// TODO: this and all other
	connect(this->get_feed_list_task, SIGNAL(error(const QString&)),
		this, SIGNAL(error(const QString&)) );

	connect(this->get_feed_list_task, SIGNAL(feeds_gotten()),
		this, SLOT(get_reading_list()) );

	this->get_feed_list_task->process();
}


}}}}

