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


#ifndef GROV_HEADER_CLIENT_READER_GOOGLE_READER_TASK
#define GROV_HEADER_CLIENT_READER_GOOGLE_READER_TASK


class QNetworkRequest;

#include <grov/common.hpp>

#include "network_task.hpp"

#include "google_reader_task.hxx"


namespace grov { namespace client { namespace reader {


/// Base class for all Google Reader's tasks.
class Google_reader_task: public Network_task
{
	Q_OBJECT

	public:
		Google_reader_task(const QString& auth_id, QObject* parent = NULL);
		Google_reader_task(const QString& login, const QString& password, QObject* parent = NULL);


	protected:
		/// Google Reader's authentication id.
		QString	auth_id;

		/// Google Reader's API token.
		///
		/// Empty if not gotten yet.
		QString	token;

	private:
		/// Google Reader's login.
		QString	login;

		/// Google Reader's password.
		QString	password;


	public:
		/// Processes the task.
		virtual void			process(void);

	protected:
		/// Called when we logins to Google Reader.
		virtual void			authenticated(void) = 0;

		/// Gets Google Reader's API token.
		///
		/// This is an asynchronous operation. token_gotten() is called when we
		/// get an API token, or failed() is called on error.
		void					get_token(void);

		/// See Network_task::prepare_request().
		virtual QNetworkRequest	prepare_request(const QString& url);

		/// Called when we get Google Reader's API token.
		virtual void			token_gotten(void) {};


	private slots:
		/// Called when we successfully login Google Reader.
		void			on_authenticated(const QString& auth_id);

		/// Called when we get Google Reader's API token.
		void			on_token_gotten(const QString& token);
};


}}}

#endif

