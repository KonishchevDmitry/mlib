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


#include <src/common.hpp>
#include <src/feed_item.hxx>

#include <src/reader/implementation.hpp>

#include "reader.hpp"


namespace grov
{


Reader::Reader(Storage* storage, const QString& user, const QString& password, QObject* parent)
:
	QObject(parent),
	impl(new reader::Implementation(storage, user, password, this))
{
	this->connect(impl, SIGNAL(error(const QString&)),
		this, SIGNAL(error(const QString&)) );

	this->connect(impl, SIGNAL(reading_list_gotten()),
		this, SIGNAL(reading_list_gotten()) );
}



void Reader::get_reading_list(void)
{
	impl->get_reading_list();
}


}

