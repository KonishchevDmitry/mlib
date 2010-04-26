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


#ifndef GROV_HEADER_CLIENT_WEB_CACHE
#define GROV_HEADER_CLIENT_WEB_CACHE

class QNetworkReply;

#include <QtCore/QByteArray>
#include <QtCore/QIODevice>

#include <QtNetwork/QAbstractNetworkCache>
#include <QtNetwork/QNetworkAccessManager>

#include <grov/common.hpp>
#include <grov/client/storage.hxx>

#include "web_cache.hxx"


namespace grov { namespace client {


/// Represents a document cached by Web_cache.
class Web_cache_entry
{
	public:
		Web_cache_entry(void);

		Web_cache_entry(
			const QString& url, const QString& location,
			const QString& content_type, const QString& content_encoding
		);

		Web_cache_entry(
			const QString& url, const QString& location,
			const QString& content_type, const QString& content_encoding,
			const QByteArray& data
		);


	public:
		/// The document's URL.
		QString		url;

		/// HTTP's Location header's value.
		QString		location;

		/// HTTP's Content-Type header's value.
		QString		content_type;

		/// HTTP's Content-Encoding header's value.
		QString		content_encoding;

		/// The document's data.
		QByteArray	data;


	public:
		/// HTTP's Content-Length header's value.
		size_t	content_length(void) const;

		/// Return false if this is invalid cache item.
		bool	valid(void) const;
};



namespace Web_cache_aux {


	/// Device that Web_cache creates to provide QtWebKit a way to get a cached
	/// data.
	class Cache_device: public QIODevice
	{
		Q_OBJECT

		public:
			Cache_device(const Web_cache_entry& cache_entry, QObject* parent = NULL);
			~Cache_device(void);


		private:
			/// Cache data.
			Web_cache_entry	cache_entry;

			/// Current position on data (in the reading mode).
			size_t			cur_pos;


		public:
			/// Returns device's cache data.
			const Web_cache_entry&	get_data(void) const;


		// QIODevice interface -->
			public:
				virtual bool	atEnd(void) const;
				virtual qint64	bytesAvailable(void) const;
				virtual bool	isSequential(void) const;
				virtual qint64	pos(void) const;
				virtual bool	reset(void);
				virtual bool	seek(qint64 pos);
				virtual qint64	size(void) const;
				virtual bool	waitForReadyRead(int msecs);
				virtual bool	waitForBytesWritten(int msecs);

			protected:
				virtual qint64	readData(char* data, qint64 maxlen);
				virtual qint64	writeData(const char* data, qint64 len);
		// QIODevice interface <--
	};


}



/// Object that is used to track QtWebKit's requests and save gotten data to
/// storage in online mode and to get saved data from storage in offline mode.
///
/// @ATTENTION
///
/// This cache class is slightly different from QAbstractNetworkCache. It does
/// not really remove prepared devices in remove() method. This is done because
/// QAbstractNetworkCache does not intended for using in several requests that
/// run simultaneously and may have the same URL, but QtWebKit uses this model
/// of loading it's pages.
///
/// So this class is expected to use in the following manner:
/// - In online mode you always create a new instance of QWebPage for every page
/// that needs to load and create a new Web_cache for this QWebPage. All
/// prepared devices does not removed by remove() which results to additional
/// memory usage, but when QWebPage will be destroyed, Web_cache also will be
/// destroyed and all devices that has not been removed will be destroyed.
/// - In offline mode the cache object is works in read only mode, so there is
/// no additional memory usage.
///
/// @TODO
/// http://some/url and http://some/url#anchor is the same URLs, but our cache
/// does not know about this.
class Web_cache: public QAbstractNetworkCache
{
	Q_OBJECT

	public:
		/// Cache using mode.
		enum Mode {
			/// We can read data from the cache and write new data to it.
			MODE_ONLINE,

			/// We can only read data from the cache. All new data is ignored.
			MODE_OFFLINE
		};

	private:
		typedef Web_cache_aux::Cache_device Cache_device;


	public:
		Web_cache(Mode mode, Storage* storage);


	private:
		/// Mode in which the cache works.
		Mode			mode;

		/// Our offline data storage.
		Storage*		storage;

		/// Last requested data.
		Web_cache_entry	last_data;


	// QAbstractNetworkCache interface -->
		public:
			virtual qint64					cacheSize(void) const;
			virtual QIODevice*				data(const QUrl& qurl);
			virtual void					insert(QIODevice* device);
			virtual QNetworkCacheMetaData	metaData(const QUrl& qurl);
			virtual QIODevice*				prepare(const QNetworkCacheMetaData& metadata);
			virtual bool					remove(const QUrl& url);
			virtual void					updateMetaData(const QNetworkCacheMetaData& metadata);


		public slots:
			virtual void	clear(void);
	// QAbstractNetworkCache interface <--
};



/// Network access manager with access to cached data.
class Web_cached_manager: public QNetworkAccessManager
{
	public:
		Web_cached_manager(Storage* storage, QObject* parent = NULL);


	protected:
		virtual QNetworkReply*	createRequest(Operation op, const QNetworkRequest& req, QIODevice* outgoingData = NULL);
};


}}

#endif

