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

#include <src/common.hpp>
#include <src/common/feed_item.hxx>

#include "storage.hxx"

#include "reader/implementation.hxx"


namespace grov { namespace client {


/// Represents Google Reader as an asynchronous storage.
class Reader: public QObject
{
	Q_OBJECT

	public:
		Reader(client::Storage* storage, const QString& user, const QString& password, QObject* parent = NULL);


	private:
		/// Class implementation.
		reader::Implementation*	impl;


	public:
		/// Gets reading list.
		///
		/// This is asynchronous operation. When it will be completed either
		///  reading_list() or error() signal will be generated.
		void	get_reading_list(void);


	signals:
		/// Request failed.
		void	error(const QString& error);

		/// Emits when all reading list's items gotten.
		// TODO
		void	reading_list_gotten(void);
};


}}

#endif

