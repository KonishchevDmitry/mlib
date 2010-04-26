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


#ifndef GROV_HEADER_CLIENT_READER_TASKS_GET_READING_LIST
#define GROV_HEADER_CLIENT_READER_TASKS_GET_READING_LIST


#include <grov/common.hpp>
#include <grov/common/feed_item.hpp>

#include <grov/client/storage.hxx>
#include <grov/client/reader/google_reader_task.hpp>

#include "get_reading_list.hxx"


namespace grov { namespace client { namespace reader { namespace tasks {


/// Gets Google Reader's reading list.
class Get_reading_list: public Google_reader_task
{
	Q_OBJECT

	public:
		Get_reading_list(Storage* storage, const QString& login, const QString& password, QObject* parent = NULL);


	private:
		/// Our offline data storage.
		Storage*	storage;

		/// Reading list continuation code.
		QString		continuation_code;

		/// Gotten reading lists counter.
		size_t		reading_lists_counter;


	public:
		/// See Google_reader_task::authenticated().
		virtual void	authenticated(void);

		/// See Network_task::request_finished().
		virtual void	request_finished(QNetworkReply* reply, const QString& error, const QByteArray& data);

	private:
		/// Called when we get reading list.
		void			on_reading_list_gotten(void);


	signals:
		/// Emitted when all reading list's items gotten.
		void	reading_list_gotten(void);


	private slots:
		/// Gets reading list.
		void	get_reading_list(void);

		/// Called when all items has been downloaded.
		void	on_items_downloaded(void);
};


}}}}

#endif

