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

#include <QtCore/QSet>

#include <grov/common.hpp>

#include <grov/common/feed_item.hpp>

#include <grov/client/reader/task.hpp>
#include <grov/client/storage.hxx>

#include "download_feeds_items.hxx"


namespace grov { namespace client { namespace reader { namespace tasks {


namespace Download_feeds_items_aux {


	/// Represents a mirroring stream in which we download all item's summary's
	/// and page's data.
	class Mirroring_stream: public QObject
	{
		Q_OBJECT

		private:
			/// Current downloading state.
			enum State {
				/// We are not downloading any item at this moment.
				STATE_NONE,

				/// We are downloading item's summary at this moment.
				STATE_SUMMARY_DOWNLOADING,

				/// We are downloading item's page at this moment.
				STATE_PAGE_DOWNLOADING,

				/// Stream closed, but not destroyed yet.
				STATE_CLOSED
			};


		public:
			Mirroring_stream(Storage* storage, QObject* parent = NULL);
			~Mirroring_stream(void);


		public:
			/// Our offline data storage.
			Storage*		storage;

			/// Current downloading state.
			State			state;

			/// Our item's summary downloader.
			QWebPage*		summary_downloader;

			/// Our item's page downloader.
			QWebPage*		page_downloader;

			/// Page loading timeout timer.
			QTimer*			timeout_timer;

			/// Item that is mirroring at this moment.
			Db_feed_item	item;

			/// Is we already mirrored item's summary and now mirroring item's
			/// page.
			bool			summary_mirrored;


		public:
			/// Closes the stream.
			void	close(void);

			/// Mirrors a next feed item.
			///
			/// @return false on error.
			void	mirror_next(void);


		signals:
			/// Emitted on error.
			void	error(const QString& error);

			/// Emitted when all items are mirrored.
			void	finished(void);


		private slots:
			/// Called when page loading timeout is expired.
			void	download_timed_out(void);

			/// Called when page loading finishes.
			void	page_download_finished(bool ok);

			/// Called when summary loading finishes.
			void	summary_download_finished(bool ok);
	};


}



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


	private slots:
		/// Called when mirroring fails.
		void			stream_error(const QString& error);

		/// Called when stream finishes mirroring.
		void			stream_finished(void);
};


}}}}

#endif

