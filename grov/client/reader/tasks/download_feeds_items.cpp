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


#include <QtCore/QTimer>

#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

#include <QtWebKit/QWebFrame>

#include <grov/common.hpp>
#include <grov/common/feed_item.hpp>

#include <grov/client/storage.hpp>
#include <grov/client/web_cache.hpp>
#include <grov/client/web_page.hpp>

#include "download_feeds_items.hpp"
#include "download_feeds_items_private.hpp"


namespace grov { namespace client { namespace reader { namespace tasks {


namespace Download_feeds_items_aux {


// Logging_network_access_manager -->
	Logging_network_access_manager::Logging_network_access_manager(QObject* parent)
	:
		QNetworkAccessManager(parent)
	{
	}



	QNetworkReply* Logging_network_access_manager::createRequest(Operation op, const QNetworkRequest& req, QIODevice* outgoingData)
	{
		MLIB_D("Creating a request for the '%1'...", req.url().toString());

		QNetworkReply* reply = QNetworkAccessManager::createRequest(op, req, outgoingData);

		if(op == QNetworkAccessManager::GetOperation)
			connect( reply, SIGNAL(finished()), SLOT(reply_finished()) );

		return reply;
	}



	bool Logging_network_access_manager::is_successful(QNetworkReply* reply)
	{
		QString error;

		if(reply->error())
		{
			error = reply->errorString();

			if(error.isEmpty())
				error = "Unknown error";
		}
		else
		{
			int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

			// Simple check - we need not more.
			if(code < 200 || code >= 400)
				error = _F("Server returned error: %1 (%2).",
					reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString(), code );
		}

		{
			MLIB_STATIC_METHOD_DEBUG_WORKAROUND

			if(error.isEmpty())
				MLIB_D("Request is successful.");
			else
				MLIB_D(PAM("Request is not successful.", error));
		}

		return error.isEmpty();
	}



	void Logging_network_access_manager::reply_finished(void)
	{
		QNetworkReply* reply = m::checked_qobject_cast<QNetworkReply*>(this->sender());

		MLIB_D("Request for the '%1' finished.", reply->url().toString());

		if(this->is_successful(reply))
			emit this->url_gotten(reply->url().toString());
	}
// Logging_network_access_manager -->



// Mirroring_stream -->
	Mirroring_stream::Mirroring_stream(Storage* storage, QObject* parent)
	:
		Network_task(parent),

		storage(storage),
		state(STATE_NONE),
		downloader(NULL),
		timeout_timer(new QTimer(this))
	{
		MLIB_D("[%1] Creating...", this);

		this->set_timeout(config::page_mirroring_timeout, true);
		this->set_max_fails(0);

		this->timeout_timer->setSingleShot(true);
		this->timeout_timer->setInterval(config::page_mirroring_timeout * 1000);
		connect(this->timeout_timer, SIGNAL(timeout()),
			this, SLOT(download_timed_out()) );
	}



	Mirroring_stream::~Mirroring_stream(void)
	{
		MLIB_D("[%1] Destroying...", this);
	}



	void Mirroring_stream::close(void)
	{
		MLIB_D("[%1] Closing...", this);

		if(this->state != STATE_CLOSED)
		{
			this->state = STATE_CLOSED;

			this->destroy_downloader();
			this->timeout_timer->stop();

			emit this->finished();
			this->deleteLater();
		}
	}



	void Mirroring_stream::create_downloader(void)
	{
		MLIB_D("[%1] Creating a new downloader...", this);

		if(this->downloader)
		{
			MLIB_DW(
				"[%1] Gotten request for creating a new downloader when downloader "
				"is already exists. Destroying current downloader...", this
			);
			this->destroy_downloader();
		}

		this->downloader = new Web_page(this);

		// Our services -->
		{
			Logging_network_access_manager* manager =
				new Logging_network_access_manager(this->downloader);

			manager->setCache( new Web_cache(Web_cache::MODE_ONLINE, this->storage) );

			connect(manager, SIGNAL(url_gotten(const QString&)),
				SLOT(url_gotten(const QString&)) );

			this->downloader->setNetworkAccessManager(manager);
		}
		// Our services <--

		// Qt::QueuedConnection is to prevent a recursion in case of simple
		// item's summaries without external elements.
		connect(this->downloader, SIGNAL(loadFinished(bool)),
			this, SLOT(page_load_finished(bool)), Qt::QueuedConnection );
	}



	void Mirroring_stream::destroy_downloader(void)
	{
		if(this->downloader)
		{
			MLIB_D("Destroying current downloader...");
			disconnect(this->downloader, NULL, this, NULL);
			this->downloader->triggerAction(QWebPage::Stop);
			this->downloader->deleteLater();
			this->downloader = NULL;
		}
	}



	void Mirroring_stream::download_finished(void)
	{
		this->destroy_downloader();
		this->timeout_timer->stop();

		switch(this->state)
		{
			case STATE_SUMMARY_DOWNLOADING:
			{
				MLIB_D("[%1] Mirroring item's '%2' (%3) page...", this, this->item.title, this->item.url);

				this->state = STATE_PAGE_DOWNLOADING;
				this->timeout_timer->start();
				this->create_downloader();
				this->downloader->mainFrame()->load(this->item.url);
			}
			break;

			case STATE_PAGE_DOWNLOADING:
			{
				// Getting URLs that we must download on our own -->
					try
					{
						Q_FOREACH(const QString& url, this->gotten_urls)
							if(!this->storage->is_in_web_cache(url))
								this->needs_download.enqueue(url);
					}
					catch(m::Exception& e)
					{
						emit this->error(EE(e));
						this->close();
						return;
					}

					this->gotten_urls.clear();

					if(!this->needs_download.empty())
					{
						MLIB_D("[%1] Pages that we must download on our own:", this);

						Q_FOREACH(const QString& url, this->needs_download)
							MLIB_D("[%1] %2", this, url);
					}
				// Getting URLs that we must download on our own <--

				this->state = STATE_URLS_DOWNLOADING;
				this->downloads_limit = this->needs_download.size() * 3;
				this->download_urls();
			}
			break;

			default:
				MLIB_LE();
				break;
		}
	}



	void Mirroring_stream::download_timed_out(void)
	{
		MLIB_D("[%1] Downloading [%2] (%3) timed out.", this, this->state, this->item.url);

		switch(this->state)
		{
			case STATE_SUMMARY_DOWNLOADING:
			case STATE_PAGE_DOWNLOADING:
				this->download_finished();
				break;

			default:
				MLIB_DW("[%1] Gotten invalid timeout signal (current state is %2). Ignoring it.", this, this->state);
				break;
		}
	}



	void Mirroring_stream::download_urls(void)
	{
		MLIB_D("[%1] Downloading next URL...", this);
		MLIB_A(this->state == STATE_URLS_DOWNLOADING);

		// Simple protection from the infinite loop from the Location headers.
		// -->
			if(this->downloads_limit < 1 && !this->needs_download.empty())
			{
				MLIB_D("[%1] URLs download limit has been exceeded.", this);
				this->needs_download.clear();
			}
		// <--

		if(this->needs_download.empty())
		{
			MLIB_D("[%1] All URLs downloaded.", this);
			this->state = STATE_NONE;
			this->mirror_next();
		}
		else
		{
			QString url = this->needs_download.dequeue();
			MLIB_D("[%1] Getting '%2'.", this, url);

			this->downloads_limit--;
			this->reset_fails();
			this->get(url, false);
		}
	}



	void Mirroring_stream::mirror_next(void)
	{
		MLIB_D("[%1] Mirroring next item...", this);
		MLIB_A(this->state == STATE_NONE);

		try
		{
			this->item = this->storage->get_next_item();
			// TODO
			//this->item.url = "http://server.lab83/papercraft/test/" + QString::number(this->item.id) + "/";
			//this->item.url = "http://server.lab83/tracs/prefix/doxygen";

			MLIB_D("[%1] Mirroring item's '%2' (%3) summary...", this, this->item.title, this->item.url);
			this->state = STATE_SUMMARY_DOWNLOADING;
			this->timeout_timer->start();
			this->create_downloader();
			this->downloader->mainFrame()->setHtml(this->item.summary);
		}
		catch(Storage::No_more_items&)
		{
			this->close();
		}
		catch(Storage::No_selected_items&)
		{
			emit this->error(tr("Logical error."));
			this->close();
		}
		catch(m::Exception& e)
		{
			emit this->error(EE(e));
			this->close();
		}
	}



	void Mirroring_stream::page_load_finished(bool ok)
	{
		MLIB_D("[%1] Page loading finished(%2).", this, ok);

		if(this->downloader == m::checked_qobject_cast<QWebPage*>(this->sender()))
			this->download_finished();
		else
			MLIB_D("[%1] Gotten the out of date downloader's signal. Ignoring it.", this);
	}



	void Mirroring_stream::process(void)
	{
		MLIB_D("[%1] Starting...", this);

		switch(this->state)
		{
			case STATE_NONE:
				this->mirror_next();
				break;

			case STATE_CLOSED:
				MLIB_D("[%1] Cancel starting - stream has been closed.", this);
				break;

			default:
				MLIB_DW("[%1] Gotten invalid start signal (current state is %2). Ignoring it.", this, this->state);
				break;
		}
	}



	void Mirroring_stream::request_finished(QNetworkReply* reply, const QString& error, const QByteArray& data)
	{
		QString url = reply->url().toString();
		MLIB_D("[%1] GET request for the '%2' finished.", this, url);

		if(this->state == STATE_CLOSED)
		{
			MLIB_D("[%1] Stream is closed. Ignoring all pending tasks.", this);
			return;
		}

		try
		{
			// Checking for errors -->
				if(this->throw_if_fatal_error(error))
				{
					MLIB_D("[%1] Request failed. Trying again...", this);
					this->get(url, false);
					return;
				}
			// Checking for errors <--

			// Adding gotten URL to the cache -->
			{
				QString location;
				QString content_type;
				QString content_encoding;

				// Headers -->
					Q_FOREACH(const QByteArray& header, reply->rawHeaderList())
					{
						QString header_uni_name = header.toLower();

						if(header_uni_name == "location")
							location = reply->rawHeader(header);
						else if(header_uni_name == "content-type")
							content_type = reply->rawHeader(header);
						else if(header_uni_name == "content-encoding")
							content_encoding = reply->rawHeader(header);
					}

					if(content_type.isEmpty() && location.isEmpty())
						MLIB_D("[%1] Gotten data with empty Content-Type and Location headers.", this);
				// Headers <--

				try
				{
					// Server may redirect us to the page with unique URL but with
					// the same contents that the page which downloader got. So we
					// must download it too.
					if(!location.isEmpty() && !this->storage->is_in_web_cache(location))
						this->needs_download << location;

					this->storage->add_web_cache_entry(
						Web_cache_entry(url, location, content_type, content_encoding, data) );
				}
				catch(m::Exception& e)
				{
					emit this->error(EE(e));
					this->close();
					return;
				}
			}
			// Adding gotten URL to the cache <--
		}
		catch(m::Exception& e)
		{
			MLIB_D("[%1] Request failed. %2", this, EE(e));
		}

		this->download_urls();
	}



	void Mirroring_stream::url_gotten(const QString& url)
	{
		MLIB_D("[%1] Downloader got a '%2'.", this, url);
		this->gotten_urls << url;
	}
// Mirroring_stream <--


}



Download_feeds_items::Download_feeds_items(Storage* storage, QObject* parent)
:
	Task(parent),
	storage(storage),
	downloading_failed(false)
{
	for(int i = 0; i < config::mirroring_streams_num; i++)
	{
		Mirroring_stream* stream = new Mirroring_stream(storage, this);

		connect(this, SIGNAL(start_mirroring()),
			stream, SLOT(process()), Qt::QueuedConnection );

		connect(stream, SIGNAL(finished()),
			SLOT(stream_finished()), Qt::QueuedConnection );

		connect(stream, SIGNAL(error(const QString&)),
			SLOT(stream_error(const QString&)) );

		this->streams << stream;
	}
}



void Download_feeds_items::process(void)
{
	MLIB_D("Downloading all feeds' items' content...");

// TODO
#if 0 && GROV_OFFLINE_DEVELOPMENT
	this->finish();
	emit this->downloaded();
#else
	this->storage->set_current_source_to_all();
	emit this->start_mirroring();
#endif
}



void Download_feeds_items::stream_error(const QString& error)
{
	MLIB_D("Stream %1 error '%2'.", this->sender(), error);

	if(!this->downloading_failed)
	{
		this->downloading_failed = true;

		Q_FOREACH(Mirroring_stream* stream, this->streams)
			stream->close();

		this->failed(error);
	}
}



void Download_feeds_items::stream_finished(void)
{
	MLIB_D("Stream %1 finished.", this->sender());

	Mirroring_stream* stream =
		m::checked_qobject_cast<Mirroring_stream*>(this->sender());

	this->streams.remove(stream);

	if(this->streams.empty() && !this->downloading_failed)
	{
		this->finish();
		emit this->downloaded();
	}
}


}}}}

