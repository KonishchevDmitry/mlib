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


#ifndef GROV_HEADER_CLIENT_READER
#define GROV_HEADER_CLIENT_READER

class QNetworkCookieJar;

#include <QtCore/QQueue>

#include <src/common.hpp>

#include "storage.hxx"

#include "reader/task.hxx"

#include "reader.hxx"


namespace grov { namespace client {


/// Represents Google Reader as an asynchronous storage.
class Reader: public QObject
{
	Q_OBJECT

	private:
	// TODO
		/// Possible asynchronous tasks.
		enum Task_type {
			/// Get feed list (list of all user's feeds).
		//	TASK_TYPE_GET_FEED_LIST,

			/// Get reading list (list of items that user did not read).
			TASK_TYPE_GET_READING_LIST
		};


	public:
		Reader(client::Storage* storage, const QString& user, const QString& password, QObject* parent = NULL);


	public:
		/// Storage for offline data.
		client::Storage*	storage;

// TODO:
		/// Storage for all ours cookies.
		QNetworkCookieJar*	cookies;

		/// Google Reader's authentication id.
		QString				auth_id;

	private:
		/// Google Reader's user name.
		QString				user;

		/// Google Reader's password.
		QString				password;


// TODO
		/// Google Reader's tasks, waiting for login.
		QQueue<Task_type>	pending_gr_tasks;


	public:
	// TODO: description
		/// Gets reading list (list of items that user did not read).
		///
		/// This is asynchronous operation. When it will be completed either
		/// reading_list() or error() signal will be generated.
		void	get_offline_data(void);

	private:
		/// Adds Google Reader's task (task, that needs Google Reader login).
		void			add_google_reader_task(Task_type type);

		/// Prepares and process a task.
		void			process_task(reader::Task* task);


	signals:
		/// When user cancelling current operation or when task processing
		/// fails, we cancelling all pending and processing tasks.
		///
		/// All tasks connects to this signal.
		///
		/// There is no error in cancelling all tasks when one of them fails -
		/// for our goals this works fine.
		void	cancel_all_tasks(void);

		/// Emits when task processing fails.
		void	error(const QString& error);

		/// Emits when all reading list's items gotten.
		// TODO
		void	reading_list_gotten(void);


	private slots:
		/// Called when we successfully login to Google Reader.
		void	authenticated(const QString& auth_id);

		/// Called when task processing fails.
		void	task_error(const QString& message);
};


}}

#endif

