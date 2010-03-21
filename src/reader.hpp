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


#ifndef GROV_HEADER_READER
#define GROV_HEADER_READER

#include <src/common.hpp>
#include <src/feed_item.hxx>
#include <src/storage.hxx>

#include "reader_impl.hxx"


namespace grov
{

/// Represents Google Reader as a storage.
class Reader: public QObject
{
	Q_OBJECT

	public:
		Reader(Storage* storage, const QString& user, const QString& password, QObject* parent = NULL);


	private:
		/// Class implementation.
		Reader_impl*	impl;


	public:
		/// Gets reading list.
		///
		/// This is asynchronous operation. When it will be completed either
		///  reading_list() or error() signal will be generated.
		void			get_reading_list(void);


	signals:
		/// Request failed.
		void	error(const QString& error);

		/// Reading list gotten.
		void	reading_list(const Feed_items_list& items);
};

}

#endif

