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


#ifndef GROV_HEADER_READER_TASKS_GET_READING_LIST
#define GROV_HEADER_READER_TASKS_GET_READING_LIST


class QNetworkRequest;

#include <src/common.hpp>
#include <src/feed_item.hpp>

#include <src/reader/network_task.hpp>

#include "get_reading_list.hxx"


namespace grov { namespace reader { namespace tasks {


/// Gets Google Reader reading list.
class Get_reading_list: public Network_task
{
	Q_OBJECT

	public:
		Get_reading_list(const QNetworkRequest& request_template, QObject* parent = NULL);


	public:
		/// Processes the task.
		virtual void	process(void);

		/// See Network_task::request_finished().
		virtual void	request_finished(const QString& error, const QByteArray& reply);


	signals:
		/// Emits when next bunch of items gotten.
		void	items_gotten(const Feed_items_list& items);

		/// Emits when all reading list's items gotten.
		void	reading_list_gotten(void);
};


}}}

#endif

