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

#include <grov/common.hpp>

#include "storage.hxx"

#include "reader/task.hxx"

#include "reader.hxx"


namespace grov { namespace client {


/// Represents Google Reader as an asynchronous storage.
class Reader: public QObject
{
	Q_OBJECT

	public:
		Reader(client::Storage* storage, QObject* parent = NULL);


	public:
		/// Storage for offline data.
		client::Storage*	storage;


	public:
		/// Flushes all offline data (sends all saved user actions to Google
		/// Reader).
		///
		/// This is asynchronous operation. When it will be completed either
		/// offline_data_flushed() or error() or cancelled() signal will be
		/// emitted.
		void	flush_offline_data(const QString& login, const QString& password);

		/// Gets reading list (list of items that user did not read).
		///
		/// This is asynchronous operation. When it will be completed either
		/// reading_list_gotten() or error() or cancelled() signal will be
		/// emitted.
		void	get_reading_list(const QString& login, const QString& password);

		/// Cancels current operation.
		void	cancel(void);

	private:
		/// Prepares and process a task.
		void	process_task(reader::Task* task);


	signals:
		/// When user cancelling current operation, we cancelling all processing
		/// tasks.
		///
		/// All tasks connects to this signal.
		///
		/// There is no error in cancelling all tasks when one of them fails -
		/// for our goals this works fine.
		void	cancel_all_tasks(void);

		/// Emitted when a task cancelled.
		void	cancelled(void);

		/// Emitted when task processing fails.
		void	error(const QString& error);

		/// Emitted when we flush all offline data.
		void	offline_data_flushed(void);

		/// Emitted when we get reading list.
		void	reading_list_gotten(void);
};


}}

#endif

