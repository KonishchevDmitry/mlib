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

			/// When we are downloading Google Reader's data at this moment to
			/// go offline.
			MODE_GOING_OFFLINE,

			/// When we downloaded Google Reader's data and working in offline
			/// mode.
			MODE_OFFLINE
		};


	public:
		Client(const QString& user, const QString& password, QObject* parent = NULL);


	private:
		/// Current mode.
		Mode			mode;

		/// Our Google Reader abstraction.
		client::Reader*	reader;


	public:
		/// Returns current mode.
		Mode			current_mode(void);

		/// Deletes all offline data and goes to mode MODE_NONE.
		void			discard_offline_data(void);

		/// Goes to offline mode.
		///
		/// This is asynchronous operation.
		void			go_offline(void);

	private:
		/// Changes current mode.
		void			change_mode(Mode mode);


	signals:
		/// Called when current mode changed.
		void			mode_changed(Client::Mode mode);


	private slots:
		/// On reader request error.
		virtual void	on_reader_error(const QString& message);

		/// On reader request error.
		virtual void	on_reading_list(void);
};


}

#endif

