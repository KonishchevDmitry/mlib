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

#include <src/common.hpp>
#include <src/common/feed.hpp>

#include <src/client/reader.hpp>
#include <src/client/storage.hpp>

#include "gr_xml_parser.hpp"

#include "get_feed_list.hpp"


namespace grov { namespace client { namespace reader { namespace tasks {


Get_feed_list::Get_feed_list(Reader* reader, QObject* parent)
:
	Google_reader_task(reader, parent)
{
}



void Get_feed_list::request_finished(const QString& error, const QByteArray& reply)
{
	MLIB_D("Subscription list request finished.");

	try
	{
		// TODO
	//	{
	//		QFile reading_list("subscription_list");
	//		reading_list.open(QIODevice::WriteOnly);
	//		reading_list.write(reply);
	//		reading_list.close();
	//	}

		Gr_feed_list feeds;

		try
		{
			// Checking for errors -->
				if(this->throw_if_fatal_error(error))
				{
					MLIB_D("Request failed. Trying again...");
					this->process();
					return;
				}
			// Checking for errors <--

			// Getting feeds -->
				try
				{
					feeds = Gr_xml_parser().subscription_list(reply);
				}
				catch(m::Exception& e)
				{
					M_THROW(PAM( tr("Parsing error:"), EE(e) ));
				}
			// Getting feeds <--
		}
		catch(m::Exception& e)
		{
			M_THROW(tr("Unable to get Google Reader's subscription list. %1"), EE(e));
		}

		// Throws m::Exception
		this->reader->storage->add_feeds(feeds);

		emit this->feeds_gotten();
	}
	catch(m::Exception& e)
	{
		emit this->error(EE(e));
	}
}



void Get_feed_list::process(void)
{
	MLIB_D("Getting Google Reader's subscription list...");

#if OFFLINE_DEVELOPMENT
	QFile list("subscription_list");
	list.open(QIODevice::ReadOnly);
	QByteArray reply = list.readAll();
	list.close();

	this->request_finished("", reply);
#else
	// TODO: more params
	QString query = "https://www.google.com/reader/api/0/subscription/list?output=xml";
	this->get(query);
#endif
}


}}}}

