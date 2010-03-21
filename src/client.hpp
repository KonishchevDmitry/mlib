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


#ifndef GROV_HEADER_CLIENT
#define GROV_HEADER_CLIENT

#include <src/common.hpp>
#include <src/feed_item.hxx>
#include <src/reader.hxx>
#include <src/storage.hpp>

// TODO
#include <src/feed_item.hpp>

#include "client.hxx"


namespace grov
{

/// Represents our Google Reader offline client.
class Client: public Storage
{
	Q_OBJECT

	public:
		/// Mode in which client currently working.
		enum Mode {
			/// When we have no Google Reader's data.
			NONE,

			/// When we downloaded Google Reader's data and working in offline
			/// mode.
			OFFLINE
		};


	public:
		Client(const QString& user, const QString& password, QObject* parent = NULL);


	private:
		/// Our Google Reader abstraction.
		Reader*		reader;

// TODO
		/// All offline data.
//		Storage*	storage;
// TODO
Feed_items_list items;


	public:
		/// Downloads all items that user did not read.
		///
		/// This is asynchronous operation.
		void			download(void);


	signals:
		/// Called when current mode changed.
		void			mode_changed(Client::Mode mode);


	private slots:
		/// On reader request error.
		virtual void	on_reader_error(const QString& error);

		/// On reader request error.
		virtual void	on_reading_list(const Feed_items_list& items);
};

}

#endif

