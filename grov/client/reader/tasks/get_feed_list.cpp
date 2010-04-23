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
#include <src/common/feed.hpp>
#include <src/main.hpp>

#include <src/client/storage.hpp>

#include "gr_xml_parser.hpp"

#include "get_feed_list.hpp"


namespace grov { namespace client { namespace reader { namespace tasks {


Get_feed_list::Get_feed_list(Storage* storage, const QString& auth_id, QObject* parent)
:
	Google_reader_task(auth_id, parent),
	storage(storage)
{
}



void Get_feed_list::authenticated(void)
{
	MLIB_D("Getting Google Reader's subscription list...");

#if GROV_OFFLINE_DEVELOPMENT
	QFile list("subscription.list");

	if(list.open(QIODevice::ReadOnly))
		this->request_finished("", list.readAll());
	else
		this->request_finished(_F("Error while reading '%1'.", list.fileName()), "");
#else
	QString url =
		"https://www.google.com/reader/api/0/subscription/list?output=xml"
		"&client=" + QUrl::toPercentEncoding(get_user_agent());
	this->get(url);
#endif
}



void Get_feed_list::request_finished(const QString& error, const QByteArray& reply)
{
	MLIB_D("Subscription list request finished.");

	try
	{
		Gr_feed_list feeds;

		try
		{
			// Checking for errors -->
				if(this->throw_if_fatal_error(error))
				{
					MLIB_D("Request failed. Trying again...");
					this->authenticated();
					return;
				}
			// Checking for errors <--

		#if GROV_DEVELOP_MODE && !GROV_OFFLINE_DEVELOPMENT
			// For offline development -->
			{
				QFile list("subscription.list");
				list.open(QIODevice::WriteOnly);
				list.write(reply);
			}
			// For offline development <--
		#endif

			// Getting feeds -->
				try
				{
					feeds = Gr_xml_parser().subscription_list(reply);
				}
				catch(m::Exception& e)
				{
					M_THROW(PAM( tr("Parsing error."), EE(e) ));
				}
			// Getting feeds <--
		}
		catch(m::Exception& e)
		{
			M_THROW(PAM( tr("Unable to get Google Reader's subscription list."), EE(e) ));
		}

		// Throws m::Exception
		this->storage->add_feeds(feeds);

		this->finish();
		emit this->feeds_gotten();
	}
	catch(m::Exception& e)
	{
		this->failed(EE(e));
	}
}


}}}}

