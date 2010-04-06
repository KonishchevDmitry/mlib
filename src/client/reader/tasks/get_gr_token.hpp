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

#include <src/client/reader.hxx>
#include <src/client/reader/google_reader_task.hpp>

#include "get_gr_token.hxx"


namespace grov { namespace client { namespace reader { namespace tasks {


/// Gets Google Reader's API token.
class Get_gr_token: public Google_reader_task
{
	Q_OBJECT

	public:
		Get_gr_token(Reader* reader, QObject* parent = NULL);


	public:
		/// Processes the task.
		virtual void	process(void);

		/// See Network_task::request_finished().
		virtual void	request_finished(const QString& error, const QByteArray& reply);


	signals:
		/// Emitted when we get token.
		void	token_gotten(const QString& token);
};


}}}}

#endif
