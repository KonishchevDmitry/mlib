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
#include <src/common/feed_item.hxx>

#include "client/reader.hxx"
#include "client/storage.hpp"

#include "client.hxx"


namespace grov {


/// Represents our Google Reader offline client.
class Client: public client::Storage
{
	Q_OBJECT

	public:
		/// Mode in which client currently working.
		enum Mode {
			/// When we have no Google Reader's data.
			MODE_NONE,

			/// When we downloaded Google Reader's data and working in offline
			/// mode.
			MODE_OFFLINE
		};


	public:
		Client(const QString& user, const QString& password, QObject* parent = NULL);


	private:
		/// Our Google Reader abstraction.
		client::Reader*	reader;


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
		virtual void	on_reading_list(void);
};


}

#endif

