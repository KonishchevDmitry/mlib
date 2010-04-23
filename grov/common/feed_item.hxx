/**************************************************************************
*                                                                         *
*   Grov - Google Reader offline viewer                                   *
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


#ifndef GROV_HEADER_COMMON_FEED_ITEM_FWD
#define GROV_HEADER_COMMON_FEED_ITEM_FWD

#include <grov/common.hpp>

namespace grov {

	class Changed_feed_item;
	typedef QList<Changed_feed_item> Changed_feed_item_list;

	class Gr_feed_item;
	typedef QList<Gr_feed_item> Gr_feed_item_list;

	class Db_feed_item;
	typedef QList<Db_feed_item> Db_feed_item_list;

}

#endif

