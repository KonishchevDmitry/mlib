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


#include <QtNetwork/QNetworkRequest>

#include <src/common.hpp>
#include <src/common/feed_item.hpp>

#include "items_list_parser.hpp"

#include "get_reading_list.hpp"


namespace grov { namespace client { namespace reader { namespace tasks {


Get_reading_list::Get_reading_list(const QNetworkRequest& request_template, QObject* parent)
:
	Network_task(request_template, parent)
{
}



void Get_reading_list::request_finished(const QString& error, const QByteArray& reply)
{
	MLIB_D("Reading list request finished.");

	try
	{
		// Checking for errors -->
			if(!error.isEmpty())
			{
				if(this->to_many_tries())
					M_THROW(error);
				else
				{
					MLIB_D("Reading list request failed. Trying again...");
					this->process();
					return;
				}
			}
		// Checking for errors <--

		// Getting feeds' items -->
		{
			Feed_items_list items;

			try
			{
				items = Items_list_parser().parse(reply);
			}
			catch(m::Exception& e)
			{
				M_THROW(PAM( tr("Parsing error:"), EE(e) ));
			}

			emit this->items_gotten(items);
			// TODO
			emit this->reading_list_gotten();
		}
		// Getting feeds' items <--
	}
	catch(m::Exception& e)
	{
		MLIB_D("Getting reading list error. %1", EE(e));
		emit this->error(_F( tr("Unable to get Google Reader's reading list. %1"), EE(e) ));
	}
}



void Get_reading_list::process(void)
{
	MLIB_D("Getting Google Reader's reading list...");
	// TODO
	//this->get("http://www.google.com/reader/atom/user/-/state/com.google/reading-list?n=2000");
	this->get("http://www.google.com/reader/atom/user/14394675015157700687/state/com.google/reading-list?n=20");
}


}}}}

