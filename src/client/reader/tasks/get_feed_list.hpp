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


#ifndef GROV_HEADER_CLIENT_READER_TASKS_GET_FEED_LIST
#define GROV_HEADER_CLIENT_READER_TASKS_GET_FEED_LIST

#include <src/common.hpp>

#include <src/client/storage.hxx>
#include <src/client/reader/google_reader_task.hpp>

#include "get_feed_list.hxx"


namespace grov { namespace client { namespace reader { namespace tasks {


/// Gets Google Reader's subscription list.
class Get_feed_list: public Google_reader_task
{
	Q_OBJECT

	public:
		Get_feed_list(Storage* storage, const QString& auth_id, QObject* parent = NULL);


	private:
		/// Our offline data storage.
		Storage*	storage;


	public:
		/// See Google_reader_task::authenticated().
		virtual void	authenticated(void);

		/// See Network_task::request_finished().
		virtual void	request_finished(const QString& error, const QByteArray& reply);


	signals:
		/// Emitted when we get all feeds.
		void	feeds_gotten(void);
};


}}}}

#endif

