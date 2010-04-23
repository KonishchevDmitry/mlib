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


#ifndef GROV_HEADER_CLIENT_WEB_CACHE
#define GROV_HEADER_CLIENT_WEB_CACHE

class QNetworkReply;

#include <QtCore/QByteArray>
#include <QtCore/QHash>
#include <QtCore/QIODevice>

#include <QtNetwork/QAbstractNetworkCache>
#include <QtNetwork/QNetworkAccessManager>

#include <src/common.hpp>
#include <src/client/storage.hxx>

#include "web_cache.hxx"


namespace grov { namespace client {


/// Represents a document cached by Web_cache.
class Web_cache_entry
{
	public:
		Web_cache_entry(void);
		Web_cache_entry(const QString& url, const QString& content_type);
		Web_cache_entry(const QString& url, const QString& content_type, const QByteArray& data);


	public:
		/// The document's URL.
		QString		url;

		/// HTTP's Content-Type header's value.
		QString		content_type;

		/// The document's data.
		QByteArray	data;


	public:
		/// HTTP's Content-Length header's value.
		size_t	content_length(void) const;

		/// Return false if this is invalid cache item.
		bool	is_valid(void) const;
};



namespace Web_cache_aux {


	/// Device that Web_cache creates to provide QtWebKit a way to get cached
	/// data.
	class Cache_device: public QIODevice
	{
		Q_OBJECT

		public:
			/// @param write - is device creating for writing or for reading.
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
class Web_cache: public QAbstractNetworkCache
{
	Q_OBJECT

	private:
		typedef Web_cache_aux::Cache_device Cache_device;


	public:
		Web_cache(Storage* storage, QObject* parent = NULL);
		~Web_cache(void);


	private:
		/// Our offline data storage.
		Storage*		storage;

		/// Last requested data.
		Web_cache_entry	last_data;

		/// Devices that has been created by prepare().
		QHash<QString, Cache_device*>	prepared_devices;


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



/// Network access manager with caching facilities.
class Web_cached_manager: public QNetworkAccessManager
{
	public:
		Web_cached_manager(Storage* storage, QObject* parent = NULL);


	protected:
		virtual QNetworkReply*	createRequest(Operation op, const QNetworkRequest& req, QIODevice* outgoingData = NULL);
};


}}

#endif

