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


#ifndef GROV_HEADER_READER_IMPL
#define GROV_HEADER_READER_IMPL

class QNetworkAccessManager;
class QNetworkRequest;
class QNetworkReply;
class QSslError;

#include <QtCore/QQueue>
#include <QtCore/QtContainerFwd>

#include <src/common.hpp>
#include <src/feed_item.hxx>
#include <src/storage.hxx>

#include "reader_impl.hxx"


namespace grov {

class Reader_impl: public QObject
{
	Q_OBJECT

	private:
		/// Represents an asynchronous task.
		class Task // TODO: private m::Virtual_noncopyable
		{
			public:
				/// Possible asynchronous tasks.
				enum Type {
					/// Get reading list (list of items that user did not read).
					GET_READING_LIST
				};


			public:
				Task(Type type)
				: type_(type), tries(0) {}


			private:
				/// Task type.
				Type	type_;

				/// Number of attempts, which have been taken to accomplish
				/// this task.
				size_t	tries;


			public:
				/// Report about yet another failed attempt to perform this
				/// task.
				///
				/// @return true if we already have no free attempts.
				bool	fail(void);

				/// Returns a task type.
				Type	type(void) { return this->type_; };
		};


	public:
		Reader_impl(Storage* storage, const QString& user, const QString& password, QObject* parent = NULL);


	private:
		/// Storage for offline data.
		Storage*				storage;


		/// Google Reader's user name.
		QString					user;

		/// Google Reader's password.
		QString					password;


		/// Is authentication process finished.
		bool					authenticated;

		/// Google Reader's session id.
		QString					sid;


		/// Object through which we carry out the networking.
		QNetworkAccessManager*	manager;


		/// Pending tasks.
		QQueue<Task>			tasks;


	public:
		/// Gets reading list (list of items that user did not read).
		///
		/// This is asynchronous operation. When it will be completed either
		/// reading_list() or error() signal will be generated.
		void			get_reading_list(void);

	private:
		/// Starts authentication process.
		///
		/// @return true if we are already authenticated.
		bool			authenticate(void);

		/// Checks QNetworkReply's HTTP status code.
		///
		/// @throw m::Exception
		void			check_reply_status(QNetworkReply* reply);

		/// Returns a QNetworkRequest object with the common HTTP headers
		/// setted.
		QNetworkRequest	prepare_request(void);

		/// Processing pending asynchronous operations.
		void			process(void);

		/// Adds asynchronous operation to process queue.
		void			queue(const Task& task);


	signals:
		/// Request failed.
		void	error(const QString& error);

		/// Reading list gotten.
		void	reading_list(const Feed_items_list& items);


	private slots:
		/// On authentication request finished.
		void	on_authentication_finished(void);

		/// On data query finished.
		void	on_query_finished(void);

		// TODO
		void	on_data_gotten(qint64 size, qint64 total_size);
};

}

#endif

