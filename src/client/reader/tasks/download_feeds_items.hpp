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


#ifndef GROV_HEADER_CLIENT_READER_TASKS_DOWNLOAD_FEEDS_ITEMS
#define GROV_HEADER_CLIENT_READER_TASKS_DOWNLOAD_FEEDS_ITEMS

class QTimer;
class QWebPage;

#include <src/common.hpp>

#include <src/client/reader/task.hpp>
#include <src/client/storage.hxx>
#include <src/client/web_cache.hxx>

#include "download_feeds_items.hxx"


namespace grov { namespace client { namespace reader { namespace tasks {


/// Downloads all feeds' items' content: images, styles, original page, etc.
class Download_feeds_items: public Task
{
	Q_OBJECT

	public:
		Download_feeds_items(Storage* storage, QObject* parent = NULL);


	private:
		/// Our offline data storage.
		Storage*	storage;

		/// Cache to which we will save data.
		Web_cache*	cache;

		/// Our feed item downloader.
		QWebPage*	web_page;

		/// Page loading timeout timer.
		QTimer*		timeout_timer;


	public:
		/// Processes the task.
		virtual void	process(void);

	private:
		/// Mirrors a next feed item.
		void			mirror_next(void);


	signals:
		/// This signal is emitted when we have downloaded all items.
		void	downloaded(void);


	private slots:
		/// Called when page loading finishes.
		void	page_load_finished(bool ok);

		/// Called when page loading timeout is expired.
		void	page_loading_timed_out(void);
};


}}}}

#endif

