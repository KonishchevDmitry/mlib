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
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
**************************************************************************/


#include <QtCore/QDateTime>
#include <QtCore/QLocale>
#include <QtCore/QUrl>

#include <QtNetwork/QNetworkCacheMetaData>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

#include <grov/common.hpp>
#include <grov/client/storage.hpp>

#include "web_cache.hpp"


namespace grov { namespace client {


namespace Web_cache_aux {


	Cache_device::Cache_device(const Web_cache_entry& cache_entry, QObject* parent)
	:
		cache_entry(cache_entry),
		cur_pos(0)
	{
		MLIB_DV("Created for '%1'.", cache_entry.url);
	}



	Cache_device::~Cache_device(void)
	{
		MLIB_DV("Destroyed for '%1'.", cache_entry.url);
	}



	bool Cache_device::atEnd() const
	{
		return !this->bytesAvailable();
	}



	qint64 Cache_device::bytesAvailable() const
	{
		return this->size() - this->cur_pos;
	}



	const Web_cache_entry& Cache_device::get_data(void) const
	{
		return this->cache_entry;
	}



	bool Cache_device::isSequential() const
	{
		// I could not make QtWebKit work with isSequential() == true. :)
		return false;
	}



	qint64 Cache_device::pos(void) const
	{
		return this->cur_pos;
	}



	bool Cache_device::reset(void)
	{
		this->cur_pos = 0;
		return true;
	}



	bool Cache_device::seek(qint64 pos)
	{
		if(pos > this->size())
			return false;

		this->cur_pos = pos;
		return true;
	}



	qint64 Cache_device::size(void) const
	{
		return this->cache_entry.data.size();
	}



	qint64 Cache_device::readData(char *data, qint64 maxlen)
	{
		if(this->atEnd())
		{
			MLIB_DV("EOF.");
			return -1;
		}

		qint64 read_data = qMin(this->bytesAvailable(), qMax(qint64(0), maxlen));
		memcpy(data, this->cache_entry.data.constData() + this->cur_pos, read_data);
		this->cur_pos += read_data;

		MLIB_DV("Read %1 bytes of data from the '%2'.", read_data, this->cache_entry.url);

		return read_data;
	}



	bool Cache_device::waitForReadyRead(int msecs)
	{
		return true;
	}



	bool Cache_device::waitForBytesWritten(int msecs)
	{
		return true;
	}



	qint64 Cache_device::writeData(const char *data, qint64 len)
	{
		MLIB_DV("Writing %1 bytes of data for the '%2'...", len, this->cache_entry.url);

		if(this->size() + len > config::max_cache_data_entry_size)
		{
			MLIB_D("Gotten too big data for the '%1' (%2). Ignoring it.",
				this->cache_entry.url, this->size() + len);
			return -1;
		}

		this->cache_entry.data.append(data, len);
		return len;
	}


}



// Web_cache -->
	Web_cache::Web_cache(Storage* storage, QObject* parent)
	:
		QAbstractNetworkCache(parent),
		storage(storage)
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



	QIODevice* Web_cache::data(const QUrl& qurl)
	{
		QString url = qurl.toString();
		MLIB_D("Returning data for the '%1'...", url);

		Web_cache_entry data;

		if(this->last_data.url == url)
			data = this->last_data;
		else
		{
			try
			{
				data = this->storage->get_web_cache_entry(url);
			}
			catch(m::Exception& e)
			{
				MLIB_SW(EE(e));
				return NULL;
			}
		}

		if(data.is_valid())
		{
			Cache_device* device = new Cache_device(data);
			device->open(QIODevice::ReadOnly);
			return device;
		}
		else
			return NULL;
	}



	void Web_cache::insert(QIODevice* device)
	{
		// TODO: think about database transactions
		Cache_device* cache_device = m::checked_qobject_cast<Cache_device*>(device);
		QString url = cache_device->get_data().url;

		MLIB_D("Request for inserting data of %1 bytes for the '%2'.",
			cache_device->get_data().data.size(), url );

		this->prepared_devices.remove(url);
		this->storage->add_web_cache_entry(cache_device->get_data());
		cache_device->deleteLater();
	}



	QNetworkCacheMetaData Web_cache::metaData(const QUrl& qurl)
	{
		QString url = qurl.toString();
		MLIB_D("Returning metadata for the '%1'...", url);


		Web_cache_entry data;

		if(this->last_data.url == url)
			data = this->last_data;
		else
		{
			try
			{
				data = this->storage->get_web_cache_entry(url);
				this->last_data = data;
			}
			catch(m::Exception& e)
			{
				MLIB_SW(EE(e));
				return QNetworkCacheMetaData();
			}
		}


		QNetworkCacheMetaData metadata;

		if(!data.is_valid())
		{
			MLIB_D("There is no metadata for the '%1'.", url);
			return metadata;
		}

		metadata.setUrl(url);
		metadata.setSaveToDisk(true);

		QNetworkCacheMetaData::RawHeaderList headers;

		// Setting fake date -->
		{
			QDateTime current_date = QDateTime::currentDateTime().toUTC();

			QByteArray http_date;
			QLocale locale(QLocale::C);
			http_date += locale.toString(current_date, "ddd, dd MMM yyyy");
			http_date += current_date.toString(" hh:mm:ss");
			http_date += " GMT";

			headers << QNetworkCacheMetaData::RawHeader("Date", http_date);
			metadata.setExpirationDate(current_date.addYears(1));
		}
		// Setting fake date <--

		if(!data.content_type.isEmpty())
			headers << QNetworkCacheMetaData::RawHeader("Content-Type", data.content_type.toAscii());

		headers << QNetworkCacheMetaData::RawHeader("Content-Length", QString::number(data.content_length()).toAscii());
		metadata.setRawHeaders(headers);

		QNetworkCacheMetaData::AttributesMap attributes;
		attributes[QNetworkRequest::HttpStatusCodeAttribute] = 200;
		attributes[QNetworkRequest::HttpReasonPhraseAttribute] = "OK";
		attributes[QNetworkRequest::SourceIsFromCacheAttribute] = true;
		metadata.setAttributes(attributes);

		return metadata;
	}



	QIODevice* Web_cache::prepare(const QNetworkCacheMetaData& metadata)
	{
		QString url = metadata.url().toString();
		MLIB_D("Preparing device for the '%1'...", url);

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
				MLIB_D(
					"Gotten request for saving data with empty Content-Type "
					"header in the Web cache for the '%1'.", url
				);
			}
		// Content type <--

		if(this->prepared_devices.contains(url))
		{
			MLIB_SW(_F(tr(
				"Gotten request for saving data in the Web cache for the '%1' for which meta data "
				"has been already prepared but not inserted or closed. Ignoring it."), url
			));
			return NULL;
		}

		Cache_device* device = new Cache_device( Web_cache_entry(url, content_type), this );
		device->open(QIODevice::WriteOnly);
		this->prepared_devices[url] = device;
		return device;
	}



	bool Web_cache::remove(const QUrl& url)
	{
		QString url_string = url.toString();

		MLIB_D("Request for removing prepared meta data for the '%1'.", url_string);

		MLIB_ITER_TYPE(this->prepared_devices) it = this->prepared_devices.find(url_string);
		if(it != this->prepared_devices.end())
		{
			(*it)->deleteLater();
			this->prepared_devices.erase(it);
			return true;
		}
		else
		{
			MLIB_D(
				"Gotten request for removing data prepared for Web cache saving "
				"for the '%1', that has not been prepared yet. Ignoring it.", url_string
			);
			return false;
		}
	}



	void Web_cache::updateMetaData(const QNetworkCacheMetaData& metadata)
	{
		MLIB_D("Request for updating meta data for the '%1'. Ignoring it.", metadata.url().toString());
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



// Web_cached_manager -->
	Web_cached_manager::Web_cached_manager(Storage* storage, QObject* parent)
	:
		QNetworkAccessManager(parent)
	{
		this->setCache(new Web_cache(storage, this));
	}



	QNetworkReply* Web_cached_manager::createRequest(Operation op, const QNetworkRequest& req, QIODevice* outgoingData)
	{
		QNetworkRequest request = req;
		// I could not make QtWebKit work with QNetworkRequest::AlwaysCache. :)
		request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
		request.setAttribute(QNetworkRequest::CacheSaveControlAttribute, true);
		return QNetworkAccessManager::createRequest(op, request, outgoingData);
	}
// Web_cached_manager <--

}}


