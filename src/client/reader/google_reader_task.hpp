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


#ifndef GROV_HEADER_CLIENT_READER_GOOGLE_READER_TASK
#define GROV_HEADER_CLIENT_READER_GOOGLE_READER_TASK


class QNetworkRequest;

#include <src/common.hpp>

#include <src/client/reader.hxx>

#include "network_task.hpp"

#include "google_reader_task.hxx"


namespace grov { namespace client { namespace reader {


/// Base class for all Google Reader's tasks.
class Google_reader_task: public Network_task
{
	Q_OBJECT

	public:
		Google_reader_task(Reader* reader, QObject* parent = NULL);


	protected:
		/// Google Reader's API token.
		///
		/// Empty if not gotten yet.
		QString	token;


	protected:
		/// Gets Google Reader's API token.
		///
		/// This is an asynchronous operation. token_gotten() is called when we
		/// get an API token, or error() is emitted and finish() called on error.
		void					get_token(void);

		/// See Network_task::prepare_request().
		virtual QNetworkRequest	prepare_request(const QString& url);

		/// Called when we get Google Reader's API token.
		virtual void			token_gotten(void);


	private slots:
		/// Called when we get Google Reader's API token.
		void	on_token_gotten(const QString& token);
};


}}}

#endif

