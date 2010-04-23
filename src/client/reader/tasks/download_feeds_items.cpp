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


#include <QtCore/QTimer>

#include <QtWebKit/QWebFrame>
#include <QtWebKit/QWebPage>

#include <src/common.hpp>
#include <src/common/feed_item.hpp>

#include <src/client/storage.hpp>
#include <src/client/web_cache.hpp>

#include "download_feeds_items.hpp"


// TODO: tries when fails
namespace grov { namespace client { namespace reader { namespace tasks {


namespace Download_feeds_items_aux {


	Mirroring_stream::Mirroring_stream(Storage* storage, QObject* parent)
	:
		QObject(parent),
		storage(storage),
		state(STATE_NONE),
		summary_downloader(new QWebPage(this)),
		page_downloader(new QWebPage(this)),
		timeout_timer(new QTimer(this))
	{
		MLIB_D("[%1] Creating...", this);

		// QtWebKit is too bugous. But separating downloader to summary and
		// page downloader helps to avoid some bugs which results Segmentation
		// faults.
		// -->
			this->summary_downloader->networkAccessManager()->setCache(
				new Web_cache(storage, this->summary_downloader) );
			this->page_downloader->networkAccessManager()->setCache(
				new Web_cache(storage, this->page_downloader) );

			// Qt::QueuedConnection is to prevent a recursion in case of simple
			// item's summaries without external elements.
			connect(this->summary_downloader, SIGNAL(loadFinished(bool)),
				this, SLOT(summary_download_finished(bool)), Qt::QueuedConnection );
			connect(this->page_downloader, SIGNAL(loadFinished(bool)),
				this, SLOT(page_download_finished(bool)), Qt::QueuedConnection );
		// <--

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
		if(this->state != STATE_CLOSED)
		{
			this->state = STATE_CLOSED;
			emit this->finished();
			this->deleteLater();
		}
	}



	void Mirroring_stream::download_timed_out(void)
	{
		MLIB_D("[%1] Downloading [%2] (%3) timed out.", this, this->state, this->item.url);

		if(this->state != STATE_CLOSED)
		{
			this->state = STATE_NONE;
			this->mirror_next();
		}
		else
			MLIB_D("[%1] Stream closed. Skipping all activity.", this);
	}



	void Mirroring_stream::mirror_next(void)
	{
		MLIB_D("Mirroring next item...");

		if(this->state == STATE_CLOSED)
		{
			MLIB_D("[%1] Stream closed. Skipping all activity.", this);
			return;
		}

		try
		{
			this->item = this->storage->get_next_item();
			// TODO
			this->item.url = "http://server.lab83/papercraft/test/" + QString::number(this->item.id) + "/";

			MLIB_D("[%1] Mirroring item's '%2' (%3) summary...", this, this->item.title, this->item.url);
			this->state = STATE_SUMMARY_DOWNLOADING;
			this->timeout_timer->start();
// TODO
			this->summary_download_finished(true);
//			this->summary_downloader->mainFrame()->setHtml(this->item.summary);
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



	void Mirroring_stream::page_download_finished(bool ok)
	{
		MLIB_D("[%1] Page downloading finished(%2).", this, ok);

		switch(this->state)
		{
			case STATE_CLOSED:
				MLIB_D("[%1] Stream closed. Skipping all activity.", this);
				break;

			case STATE_PAGE_DOWNLOADING:
				this->timeout_timer->stop();
				this->state = STATE_NONE;
				this->mirror_next();
				break;

			default:
				MLIB_D("[%1] Gotten an out-of-date signal (current state is %1). Ignoring it.");
				break;
		}
	}



	void Mirroring_stream::summary_download_finished(bool ok)
	{
		MLIB_D("[%1] Summary downloading finished(%2).", this, ok);

		switch(this->state)
		{
			case STATE_CLOSED:
				MLIB_D("[%1] Stream closed. Skipping all activity.", this);
				break;

			case STATE_SUMMARY_DOWNLOADING:
			{
				this->timeout_timer->stop();

				MLIB_D("[%1] Mirroring item's '%2' (%3) page...", this, this->item.title, this->item.url);
				this->state = STATE_PAGE_DOWNLOADING;
				this->timeout_timer->start();
				this->page_downloader->mainFrame()->load(this->item.url);
			}
			break;

			default:
				MLIB_D("[%1] Gotten an out-of-date signal (current state is %1). Ignoring it.");
				break;
		}
	}


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

		connect(stream, SIGNAL(finished()),
			this, SLOT(stream_finished()), Qt::QueuedConnection );

		connect(stream, SIGNAL(error(const QString&)),
			this, SLOT(stream_error(const QString&)) );

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

	// Running mirroring in all streams -->
	{
		QSet<Mirroring_stream*> streams = this->streams;
		Q_FOREACH(Mirroring_stream* stream, streams)
			stream->mirror_next();
	}
	// Running mirroring in all streams <--
#endif
}



void Download_feeds_items::stream_error(const QString& error)
{
	MLIB_D("Stream %1 error '%2'.", this->sender(), error);

	if(!this->downloading_failed)
	{
		this->downloading_failed = true;
		this->failed(error);

		Q_FOREACH(Mirroring_stream* stream, this->streams)
			stream->close();
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

