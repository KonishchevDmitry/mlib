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


#ifndef GROV_HEADER_CLIENT_READER_TASKS_DOWNLOAD_FEEDS_ITEMS_PRIVATE
#define GROV_HEADER_CLIENT_READER_TASKS_DOWNLOAD_FEEDS_ITEMS_PRIVATE

class QTimer;
class QWebPage;

#include <QtCore/QSet>
#include <QtCore/QQueue>

#include <QtNetwork/QNetworkAccessManager>

#include <grov/common.hpp>
#include <grov/common/feed_item.hpp>

#include <grov/client/storage.hxx>
#include <grov/client/reader/network_task.hpp>

#include "download_feeds_items.hxx"


namespace grov { namespace client { namespace reader { namespace tasks { namespace Download_feeds_items_aux {


/// Network access manager that logs all successful requests.
///
/// It consider as successful all requests for which
/// Logging_network_access_manager::is_successful() returns true.
class Logging_network_access_manager: public QNetworkAccessManager
{
	Q_OBJECT

	public:
		Logging_network_access_manager(QObject* parent = NULL);


	public:
		/// See QNetworkAccessManager::createRequest().
		virtual QNetworkReply*	createRequest(Operation op, const QNetworkRequest& req, QIODevice* outgoingData = 0);

		/// Checks whether request is successful.
		static bool				is_successful(QNetworkReply* reply);


	signals:
		/// Emitted when GET reply created by this manager finishes
		/// successfully.
		void	url_gotten(const QString& url);


	private slots:
		/// Called when QNetworkReply finishes.
		void	reply_finished(void);
};



/// Represents a mirroring stream in which we download all item's summary's
/// and page's data.
class Mirroring_stream: public Network_task
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

			/// We are downloading URLs that downloader did not write to the
			/// cache.
			STATE_URLS_DOWNLOADING,

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

		/// Item that is mirroring at this moment.
		Db_feed_item	item;

		/// Our item downloader.
		QWebPage*		downloader;

		/// URLs that downloader got.
		QSet<QString>	gotten_urls;

		/// URLs that downloader got but not saved to the cache, so we must to
		/// download them on our own.
		QQueue<QString>	needs_download;

		/// Page loading timeout timer.
		QTimer*			timeout_timer;


	public:
		/// Closes the stream.
		void			close(void);

	protected:
		/// See Network_task::request_finished().
		virtual void	request_finished(QNetworkReply* reply, const QString& error, const QByteArray& data);

	private:
		/// Creates a new downloader.
		void			create_downloader(void);

		/// Asynchronously destroys current downloader.
		void			destroy_downloader(void);

		/// Called when download finishes.
		void			download_finished(void);

		/// Downloads all URLs from needs_download.
		void			download_urls(void);

		/// Mirrors a next feed item.
		///
		/// @return false on error.
		void			mirror_next(void);


	signals:
		/// Emitted on error.
		void	error(const QString& error);

		/// Emitted when all items are mirrored.
		void	finished(void);


	private slots:
		/// Called when page loading timeout is expired.
		void			download_timed_out(void);

		/// Called when downloader successfully downloads a URL.
		void			url_gotten(const QString& url);

		/// Called when page loading finishes.
		void			page_load_finished(bool ok);

		/// Starts mirroring process.
		virtual void	process(void);
};


}}}}}

#endif

