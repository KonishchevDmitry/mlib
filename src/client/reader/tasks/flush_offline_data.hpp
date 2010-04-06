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


#ifndef GROV_HEADER_CLIENT_READER_TASKS_FLUSH_OFFLINE_DATA
#define GROV_HEADER_CLIENT_READER_TASKS_FLUSH_OFFLINE_DATA


#include <src/common.hpp>
#include <src/common/feed_item.hpp>

#include <src/client/reader.hxx>
#include <src/client/reader/google_reader_task.hpp>

#include "flush_offline_data.hxx"


namespace grov { namespace client { namespace reader { namespace tasks {


/// Flushes all offline data (sends all saved user actions to Google Reader).
class Flush_offline_data: public Google_reader_task
{
	Q_OBJECT

	public:
		Flush_offline_data(Reader* reader, QObject* parent = NULL);


	private:
		/// Google Reader's API token.
		QString					token;

		/// Items that had been changed by user.
		Changed_feed_item_list	changed_items;


		/// Points to the first item which needs synchronization with the
		/// database.
		Changed_feed_item_list::const_iterator	changed_items_db;

		/// Points to the first item which changes has not been flushed to
		/// Google Reader.
		Changed_feed_item_list::const_iterator	changed_items_flush;

		/// Points to the end of changed_items.
		Changed_feed_item_list::const_iterator	changed_items_end;


	public:
		/// Processes the task.
		virtual void	process(void);

		/// See Network_task::request_finished().
		virtual void	request_finished(const QString& error, const QByteArray& reply);

		/// Synchronizes flushes with database.
		///
		/// @throw m::Exception.
		void			sync_with_db(void);

	private:
		/// Flushes all offline data.
		void	flush(void);


	signals:
		/// Emited when all offline data flushed.
		void	flushed(void);


	private slots:
		/// Google Reader's API token gotten.
		void	token_gotten(const QString& token);
};


}}}}

#endif

