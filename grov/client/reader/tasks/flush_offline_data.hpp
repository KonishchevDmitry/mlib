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


#ifndef GROV_HEADER_CLIENT_READER_TASKS_FLUSH_OFFLINE_DATA
#define GROV_HEADER_CLIENT_READER_TASKS_FLUSH_OFFLINE_DATA


#include <grov/common.hpp>
#include <grov/common/feed_item.hpp>

#include <grov/client/storage.hxx>
#include <grov/client/reader/google_reader_task.hpp>

#include "flush_offline_data.hxx"


namespace grov { namespace client { namespace reader { namespace tasks {


/// Flushes all offline data (sends all saved user actions to Google Reader).
class Flush_offline_data: public Google_reader_task
{
	Q_OBJECT

	public:
		Flush_offline_data(Storage* storage, const QString& login, const QString& password, QObject* parent = NULL);


	private:
		/// Our offline data storage.
		Storage*	storage;


		/// Items that had been changed by the user.
		Changed_feed_item_list	changed_items;

		/// Points to the first item which changes has not been flushed to
		/// Google Reader.
		Changed_feed_item_list::const_iterator	flush_start;

		/// When we send request to Google Reader we post changes for items in
		/// the interval [flush_start, flush_end).
		Changed_feed_item_list::const_iterator	flush_end;


	protected:
		/// See Google_reader_task::authenticated().
		virtual void	authenticated(void);

		/// See Network_task::request_finished().
		virtual void	request_finished(QNetworkReply* reply, const QString& error, const QByteArray& data);

		/// Google Reader's API token gotten.
		virtual void	token_gotten(void);

	private:
		/// Flushes all offline data.
		void			flush(void);


	signals:
		/// Emitted when all offline data flushed.
		void	flushed(void);
};


}}}}

#endif

