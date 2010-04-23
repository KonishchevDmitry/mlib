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


#ifndef GROV_HEADER_COMMON_FEED_ITEM
#define GROV_HEADER_COMMON_FEED_ITEM

#include <grov/common.hpp>

#include "feed_item.hxx"


namespace grov {


/// Represents a RSS feed's item.
class Feed_item
{
	protected:
		Feed_item(void);
		Feed_item(const QString& url, const QString& title, const QString& summary);


	public:
		/// URL of the page with this item.
		///
		/// May be empty.
		QString		url;

		/// Title.
		QString		title;

		/// Summary text.
		QString		summary;
};



/// Represents a RSS feed's item that had been changed.
class Changed_feed_item
{
	public:
		/// Item property that had been changed.
		enum Changed_property {
			/// Read status of item.
			PROPERTY_READ,

			/// Starred status of item.
			PROPERTY_STARRED
		};


	public:
		Changed_feed_item(Big_id id, const QString& gr_id, Changed_property property, bool value);


	public:
		/// Item's id.
		Big_id				id;

		/// Item's Google Reader id.
		QString				gr_id;

		/// Property that had been changed.
		Changed_property	property;

		/// Value to which an item's property had been changed.
		bool				value;
};



/// Represents a RSS feed's item gotten from Google Reader's reading list.
class Gr_feed_item: public Feed_item
{
	public:
		Gr_feed_item(void);


	public:
		/// Item's Google Reader id.
		QString		gr_id;

		/// Item's feed's Google Reader id.
		QString		feed_gr_id;

		/// Is item from a user that our user follow.
		bool		broadcast;

		/// Is item starred.
		bool		starred;
};



/// Represents a RSS feed's item gotten from ours DB.
class Db_feed_item: public Feed_item
{
	public:
		Db_feed_item(void);

		Db_feed_item(
			Big_id id, Big_id feed_id, const QString& url,
			const QString& title, const QString& summary,
			bool broadcast, bool read, bool starred
		);


	public:
		/// Item's id in our DB.
		Big_id	id;

		/// Item's feed id.
		Big_id	feed_id;

		/// Is item broadcast.
		bool	broadcast;

		/// Is item read or unread.
		bool	read;

		/// Is item starred.
		bool	starred;


	public:
		/// Marks the item as invalid item.
		void	set_invalid(void);

		/// Return false if the item is invalid item.
		bool	valid(void);
};


}

#endif

