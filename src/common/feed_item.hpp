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
// TODO: split to 2 classes
	public:
		Feed_item(void);
		Feed_item(Big_id id, Big_id feed_id, const QString& title, const QString& summary, bool starred);


	public:
		Big_id		id;
		/// Google Reader's id.
		QString		gr_id;

		Big_id		feed_id;

		QString		title;
		QString		summary;

		// TODO
		QString		feed_name;
		QString		feed_uri;
		QStringList	labels;
		bool		starred;


	public:
		/// Marks the item as invalid item.
		void	set_invalid(void);

		/// Return false if the item is invalid item.
		bool	valid(void);
};


}

#endif

