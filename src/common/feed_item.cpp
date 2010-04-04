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

#include "feed_item.hpp"


namespace grov {


Feed_item::Feed_item(void)
:
	id(-1),
	starred(false)
{
}



Feed_item::Feed_item(Big_id id, Big_id feed_id, const QString& title, const QString& summary, bool starred)
:
	id(id),
	feed_id(feed_id),
	title(title),
	summary(summary),
	starred(starred)
{
}



void Feed_item::set_invalid(void)
{
	this->id = -1;
}


bool Feed_item::valid(void)
{
	return this->id >= 0;
}


}

