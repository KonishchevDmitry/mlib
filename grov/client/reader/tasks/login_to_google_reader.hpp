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


#ifndef GROV_HEADER_CLIENT_READER_TASKS_LOGIN_TO_GOOGLE_READER
#define GROV_HEADER_CLIENT_READER_TASKS_LOGIN_TO_GOOGLE_READER

#include <grov/common.hpp>

#include <grov/client/reader/network_task.hpp>

#include "login_to_google_reader.hxx"


namespace grov { namespace client { namespace reader { namespace tasks {


/// Logins to Google Reader.
class Login_to_google_reader: public Network_task
{
	Q_OBJECT

	public:
		Login_to_google_reader(const QString& login, const QString& password, QObject* parent = NULL);


	private:
		/// Google Reader's login.
		QString	login;

		/// Google Reader's password.
		QString	password;


	public:
		/// Processes the task.
		virtual void	process(void);

		/// See Network_task::request_finished().
		virtual void	request_finished(const QString& error, const QByteArray& reply);

	private:
		/// Gets Google Reader authentication id from its reply.
		///
		/// @throw m::Exception.
		QString	get_auth_id(const QByteArray& reply);


	signals:
		/// This signal is emitted when we successfully login to Google Reader.
		void	authenticated(const QString& auth_id);
};


}}}}

#endif

