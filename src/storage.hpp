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


#ifndef GROV_HEADER_STORAGE
#define GROV_HEADER_STORAGE

#include <memory>

#include <boost/scoped_ptr.hpp>

class QSqlDatabase;
class QSqlQuery;

#include <src/common.hpp>
#include <src/feed_item.hxx>

#include "storage.hxx"


namespace grov
{

/// Represents a storage which stores all feeds' items for offline mode.
class Storage: public QObject
{
	Q_OBJECT

	public:
		/// Class for throwing in get_next_item() and get_previous_item().
		class No_more_items {};


	public:
		/// @throw m::Exception
		Storage(QObject* parent = NULL);
		~Storage(void);


	private:
		/// Database for offline access to the feeds' items.
		boost::scoped_ptr<QSqlDatabase>	db;

		/// Current query that user processes on the database.
		std::auto_ptr<QSqlQuery>		current_query;


	public:
		/// Adds items to the storage.
		///
		/// @throw m::Exception.
		void		add_items(const Feed_items_list& items);

		/// Returns next feeds' item.
		///
		/// @throw m::Exception, No_more_items.
		Feed_item	get_next_item(void);

		/// Returns previous feeds' item.
		///
		/// @throw m::Exception, No_more_items.
		Feed_item	get_previous_item(void);

	protected:
		/// Resets current internal state - current position for
		/// get_next_item() and get_previous_item(), etc.
		void		reset(void);

	private:
		/// Creates a query that will be used to display items requested by
		/// user.
		///
		/// @throw m::Exception.
		void		create_current_query(void);

		/// Returns item corresponding to current_query.
		///
		/// @throw m::Exception, No_more_items.
		Feed_item	get_item(bool next);

		/// Executes a query.
		///
		/// @throw m::Exception.
		void		exec(QSqlQuery& query);

		/// Executes a query.
		///
		/// @throw m::Exception.
		QSqlQuery	exec(const QString& query_string);

		/// Prepares SQL query for execution.
		///
		/// @throw m::Exception.
		QSqlQuery	prepare(const QString& string);
};

}

#endif


