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
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
**************************************************************************/


#include <src/common.hpp>
// TODO
#include <src/feed_item.hpp>
#include <src/reader.hpp>
#include <src/storage.hpp>

#include "client.hpp"


namespace grov
{


Client::Client(const QString& user, const QString& password, QObject* parent)
:
	Storage(parent)
	// TODO: catch Storage exception
{
	this->reader = new Reader(this, user, password, this);

	connect(this->reader, SIGNAL(error(const QString&)),
		this, SLOT(on_reader_error(const QString&)) );

	connect(this->reader, SIGNAL(reading_list(const Feed_items_list&)),
		this, SLOT(on_reading_list(const Feed_items_list&)) );
}



void Client::download(void)
{
	this->reader->get_reading_list();
}



void Client::on_reader_error(const QString& error)
{
	// TODO
	MLIB_W(error);
	exit(1);
}



void Client::on_reading_list(const Feed_items_list& items)
{
// TODO
	Q_FOREACH(const Feed_item& item, items)
		MLIB_D(item.title);
	this->items = items;
	emit mode_changed(OFFLINE);
}


}

