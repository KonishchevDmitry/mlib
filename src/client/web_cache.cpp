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
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
**************************************************************************/

// TODO: see data in db

// TODO
#include <QtCore/QFile>
#include <QtCore/QDateTime>
#include <QtCore/QUrl>

#include <QtNetwork/QNetworkCacheMetaData>
#include <QtNetwork/QNetworkRequest>

#include <src/common.hpp>
#include <src/client/storage.hpp>

#include "web_cache.hpp"


namespace grov { namespace client {


namespace Web_cache_aux {


	Cache_device::Cache_device(const Web_cache_entry& cache_entry, QObject* parent)
	:
		cache_entry(cache_entry),
		pos(0)
	{
	}



	Cache_device::~Cache_device(void)
	{
		MLIB_D("~Cache_device()");
	}



	bool Cache_device::atEnd() const
	{
		MLIB_D("atEnd()");
// TODO
		MLIB_D("atEnd(%1)", this->pos >= size_t(this->cache_entry.data.size()));
		return this->pos >= size_t(this->cache_entry.data.size());
	}



	qint64 Cache_device::bytesAvailable() const
	{
		MLIB_D("bytesAvailable()");
		return this->cache_entry.data.size() - this->pos;
	}



	const Web_cache_entry& Cache_device::get_data(void) const
	{
		MLIB_D("get_data()");
		return this->cache_entry;
	}



	bool Cache_device::isSequential() const
	{
		MLIB_D("isSequential()");
		return true;
	}



	qint64 Cache_device::readData(char *data, qint64 maxlen)
	{
		MLIB_D("readData()");

		if(this->atEnd())
		{
#warning
//			emit readChannelFinished();
			return -1;
		}

		qint64 read_data = qMin(this->bytesAvailable(), maxlen);
		memcpy(data, this->cache_entry.data.constData() + this->pos, read_data);
		this->pos += read_data;

		// TODO
		MLIB_D("readData(%1)", read_data);
#warning
MLIB_D("%1 read", read_data);
#warning
//emit readyRead();
		return read_data;
	}



	bool Cache_device::waitForReadyRead(int msecs)
	{
		MLIB_D("waitForReadyRead()");
		return true;
	}



	bool Cache_device::waitForBytesWritten(int msecs)
	{
		MLIB_D("waitForBytesWritten()");
		return true;
	}



	qint64 Cache_device::writeData(const char *data, qint64 len)
	{
		MLIB_D("writeData()");
		// TODO: think about limits
		this->cache_entry.data.append(data, len);
		return len;
	}


}



// Web_cache -->
	Web_cache::Web_cache(Storage* storage, bool offline_mode, QObject* parent)
	:
		QAbstractNetworkCache(parent),
		storage(storage),
		// TODO
		offline_mode(offline_mode)
	{
	}



	Web_cache::~Web_cache(void)
	{
		if(!this->prepared_devices.isEmpty())
			MLIB_SW(_F( tr("Web cache: we have %1 prepared meta data that has not been inserted or removed."),
				this->prepared_devices.size() ));
	}



	qint64 Web_cache::cacheSize(void) const
	{
		MLIB_D("Cache size request. Returning 0.");
		return 0;
	}



	QIODevice* Web_cache::data(const QUrl& url)
	{
		MLIB_D("Returning data for '%1'...", url.toString());

		try
		{
			// Throws m::Exception
			// TODO
			Web_cache_entry data = this->storage->get_web_cache_entry(url.toString());

#warning
//data.data = "<html><body>It works</body></html>";
			if(data.is_valid())
			{
#warning
				static int counter = 0;
				QFile* file = new QFile("cache/" + QString::number(counter++));
				file->open(QIODevice::WriteOnly);
				file->write(data.data);
				file->close();
				file->open(QIODevice::ReadOnly);
				return file;

//				Cache_device* device = new Cache_device(data);
//				device->open(QIODevice::ReadOnly);
//#warning
//MLIB_D("%1", data.data);
//MLIB_D("%1", data.data.size());
//				return device;
			}
			else
				return NULL;
		}
		catch(m::Exception& e)
		{
			MLIB_SW(EE(e));
			return NULL;
		}
	}



	void Web_cache::insert(QIODevice* device)
	{
		// TODO: think about database transactions
		Cache_device* cache_device = m::checked_qobject_cast<Cache_device*>(device);
		QString url = cache_device->get_data().url;

		MLIB_D("Request for inserting data %1 bytes for '%2'.", cache_device->get_data().data.size(), url);

		this->prepared_devices.remove(url);
		this->storage->add_web_cache_entry(cache_device->get_data());
		cache_device->deleteLater();
	}



	QNetworkCacheMetaData Web_cache::metaData(const QUrl& url)
	{
		MLIB_D("Returning metadata for '%1'...", url.toString());

		try
		{
			// Throws m::Exception
			Web_cache_entry data = this->storage->get_web_cache_entry(url.toString());

			QNetworkCacheMetaData metadata;

			if(!data.is_valid())
			{
				MLIB_D("There is no metadata for '%1'.", url.toString());
				return metadata;
			}

			QDateTime current_date = QDateTime::currentDateTime();
			// TODO
			QDateTime expiration_date = current_date.addYears(1);

			metadata.setUrl(url);
			metadata.setExpirationDate(expiration_date);
			metadata.setSaveToDisk(true);

			// TODO
			QNetworkCacheMetaData::AttributesMap attributes;
			attributes[QNetworkRequest::HttpStatusCodeAttribute] = 200;
			attributes[QNetworkRequest::HttpReasonPhraseAttribute] = "OK";
//			attributes[QNetworkRequest::CacheLoadControlAttribute] = QNetworkRequest::AlwaysCache;
//			attributes[QNetworkRequest::CacheSaveControlAttribute] = true;
//			attributes[QNetworkRequest::SourceIsFromCacheAttribute] = true;
			metadata.setAttributes(attributes);

			QNetworkCacheMetaData::RawHeaderList headers;
	//		headers << QNetworkCacheMetaData::RawHeader("date", QDateTime::currentDateTime().toString(Qt::ISODate).toAscii());
			// TODO
			headers << QNetworkCacheMetaData::RawHeader("Date", "Tue, 13 Apr 2010 18:38:27 GMT");
//			headers << QNetworkCacheMetaData::RawHeader("Date", "Tue, 13 Apr 2010 19:11:17 GMT");
			//headers << QNetworkCacheMetaData::RawHeader("Content-Type", data.content_type.toAscii());
			headers << QNetworkCacheMetaData::RawHeader("Content-Type", "text/html");
//			headers << QNetworkCacheMetaData::RawHeader("Last-Modified","Tue, 12 Jan 2010 15:29:05 GMT");
//			headers << QNetworkCacheMetaData::RawHeader("Content-Encoding","gzip");
#warning
//			headers << QNetworkCacheMetaData::RawHeader("Content-Length", "4963");
			headers << QNetworkCacheMetaData::RawHeader("Content-Length", QString::number(data.content_length()).toAscii());
#warning
MLIB_D("%1", QString::number(data.content_length()).toAscii());
			metadata.setRawHeaders(headers);

			return metadata;
		}
		catch(m::Exception& e)
		{
			MLIB_SW(EE(e));
			return QNetworkCacheMetaData();
		}
	}



	QIODevice* Web_cache::prepare(const QNetworkCacheMetaData& metadata)
	{
		QString url = metadata.url().toString();
		MLIB_D("Preparing device for '%1'...", url);

		if(url.isEmpty())
		{
			MLIB_SW(tr("Gotten request for saving data in the Web cache for empty URL. Ignoring it."));
			return NULL;
		}

		// Content type -->
			QString content_type;

			Q_FOREACH(const QNetworkCacheMetaData::RawHeader& header, metadata.rawHeaders())
			{
				if(header.first == "Content-Type")
				{
					content_type = header.second;
					break;
				}
			}

			if(content_type.isEmpty())
			{
				MLIB_SW(_F(tr("Gotten request for saving data with empty Content-Type header in the cache for '%1'. Ignoring it."), url));
				return NULL;
			}
		// Content type <--

		if(this->prepared_devices.contains(url))
		{
			MLIB_SW(_F(tr(
				"Gotten request for saving data in the cache for '%1' for which meta data "
				"has been already prepared but not inserted or closed. Ignoring it."), url
			));
			return NULL;
		}

		Cache_device* device = new Cache_device( Web_cache_entry(url, content_type), this );
		this->prepared_devices[url] = device;
		// TODO
		qDebug() << metadata.attributes();
		// TODO
		qDebug() << metadata.rawHeaders();
		device->open(QIODevice::WriteOnly);
		return device;
	}



	bool Web_cache::remove(const QUrl& url)
	{
		QString url_string = url.toString();

		MLIB_D("Request for removing prepared meta data for '%1'.", url_string);

		MLIB_ITER_TYPE(this->prepared_devices) it = this->prepared_devices.find(url_string);
		if(it != this->prepared_devices.end())
		{
			(*it)->deleteLater();
			this->prepared_devices.erase(it);
			return true;
		}
		else
		{
			MLIB_SW(_F(tr(
				"Gotten request for removing prepared for Web cache saving data for '%1', "
				"that has not been prepared yet. Ignoring it."), url_string
			));
			return false;
		}
	}



	void Web_cache::updateMetaData(const QNetworkCacheMetaData& metadata)
	{
		MLIB_D("Request for updating meta data for '%1'. Ignoring it.", metadata.url().toString());
	}



	void Web_cache::clear(void)
	{
		MLIB_D("Cache clear request. Ignoring it.");
	}
// Web_cache <--



// Web_cache_entry -->
	Web_cache_entry::Web_cache_entry(void)
	{
	}



	Web_cache_entry::Web_cache_entry(const QString& url, const QString& content_type)
	:
		url(url),
		content_type(content_type)
	{
	}



	Web_cache_entry::Web_cache_entry(const QString& url, const QString& content_type, const QByteArray& data)
	:
		url(url),
		content_type(content_type),
		data(data)
	{
	}



	size_t Web_cache_entry::content_length(void) const
	{
		return this->data.size();
	}


	bool Web_cache_entry::is_valid(void) const
	{
		return !this->url.isEmpty();
	}
// Web_cache_entry <--

}}


