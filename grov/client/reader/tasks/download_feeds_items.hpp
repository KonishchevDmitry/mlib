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


#ifndef GROV_HEADER_CLIENT_READER_TASKS_DOWNLOAD_FEEDS_ITEMS
#define GROV_HEADER_CLIENT_READER_TASKS_DOWNLOAD_FEEDS_ITEMS

#include <QtCore/QSet>

#include <grov/common.hpp>
#include <grov/client/reader/task.hpp>
#include <grov/client/storage.hxx>

#include "download_feeds_items.hxx"


namespace grov { namespace client { namespace reader { namespace tasks {


/// Downloads all feeds' items' content: images, styles, original page, etc.
class Download_feeds_items: public Task
{
	Q_OBJECT

	private:
		typedef Download_feeds_items_aux::Mirroring_stream Mirroring_stream;


	public:
		Download_feeds_items(Storage* storage, QObject* parent = NULL);


	private:
		/// Our offline data storage.
		Storage*				storage;

		/// Mirroring streams.
		QSet<Mirroring_stream*>	streams;

		/// Is mirroring failed.
		bool					downloading_failed;


	public:
		/// Processes the task.
		virtual void	process(void);


	signals:
		/// This signal is emitted when we have downloaded all items.
		void			downloaded(void);

		/// This signal is emitted in process() to asynchronously start all
		/// mirroring streams.
		void			start_mirroring(void);


	private slots:
		/// Called when mirroring fails.
		void			stream_error(const QString& error);

		/// Called when stream finishes mirroring.
		void			stream_finished(void);
};


}}}}

#endif

