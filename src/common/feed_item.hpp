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


#ifndef GROV_HEADER_COMMON_FEED_ITEM
#define GROV_HEADER_COMMON_FEED_ITEM

#include <src/common.hpp>

#include "feed_item.hxx"


namespace grov {


/// Represents a RSS feed item.
class Feed_item
{
	protected:
		Feed_item(void);
		Feed_item(const QString& title, const QString& summary);


	public:
		/// Title.
		QString		title;

		/// Summary text.
		QString		summary;
};


/// Represents a RSS feed item gotten from Google Reader's.
class Gr_feed_item: public Feed_item
{
		// TODO
	public:
		QString		feed_name;
		QString		feed_uri;
		QStringList	labels;
};


/// Represents a RSS feed item gotten from ours DB.
class Db_feed_item: public Feed_item
{
	public:
		Db_feed_item(void);
		// TODO: read
		Db_feed_item(Big_id id, const QString& title, const QString& summary, bool starred);


	public:
		/// Item's id in our DB.
		Big_id	id;

		/// Is item read or unread.
		bool	read;

		/// Is item starred.
		bool	starred;
};


}

#endif

