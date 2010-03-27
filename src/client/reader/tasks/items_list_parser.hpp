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


#ifndef GROV_HEADER_CLIENT_READER_TASKS_ITEMS_LIST_PARSER
#define GROV_HEADER_CLIENT_READER_TASKS_ITEMS_LIST_PARSER

#include <src/common.hpp>
#include <src/common/feed_item.hxx>


namespace grov { namespace client { namespace reader { namespace tasks {


/// Parses a Google Reader feeds' items list.
class Items_list_parser: public QObject
{
	Q_OBJECT

	public:
		/// @throw m::Exception.
		Feed_items_list	parse(const QByteArray& data);
};


}}}}

#endif

