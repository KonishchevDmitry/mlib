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
{
}



Feed_item::Feed_item(const QString& url, const QString& title, const QString& summary)
:
	url(url),
	title(title),
	summary(summary)
{
}



Changed_feed_item::Changed_feed_item(Big_id id, const QString& gr_id, Changed_property property, bool value)
:
	id(id),
	gr_id(gr_id),
	property(property),
	value(value)
{
}



Gr_feed_item::Gr_feed_item(void)
:
	broadcast(false),
	starred(false)
{
}



Db_feed_item::Db_feed_item(void)
:
	id(-1)
{
}



Db_feed_item::Db_feed_item(
	Big_id id, Big_id feed_id, const QString& url,
	const QString& title, const QString& summary,
	bool broadcast, bool read, bool starred
)
:
	Feed_item(url, title, summary),
	id(id),
	feed_id(feed_id),
	broadcast(broadcast),
	read(read),
	starred(starred)
{
}



void Db_feed_item::set_invalid(void)
{
	this->id = -1;
}



bool Db_feed_item::valid(void)
{
	return this->id >= 0;
}


}

