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
		cache(new Web_cache(storage, this)),
		web_page(new QWebPage(this)),
		timeout_timer(new QTimer(this))
	{
		MLIB_D("[%1] Creating...", this);

		this->web_page->networkAccessManager()->setCache(this->cache);

		// Qt::QueuedConnection is to prevent a recursion in case of simple
		// item's summaries without external elements.
		connect(this->web_page, SIGNAL(loadFinished(bool)),
			this, SLOT(page_load_finished(bool)), Qt::QueuedConnection );

		this->timeout_timer->setSingleShot(true);
		this->timeout_timer->setInterval(config::page_mirroring_timeout * 1000);
		connect(this->timeout_timer, SIGNAL(timeout()),
			this, SLOT(page_loading_timed_out()) );
	}



	Mirroring_stream::~Mirroring_stream(void)
	{
		MLIB_D("[%1] Destroying...", this);
	}



	bool Mirroring_stream::mirror_next(void)
	{
		try
		{
			this->item = this->storage->get_next_item();
			#warning
			this->item.url = "http://server.lab83/papercraft/test/" + QString::number(this->item.id) + "/";
			//this->item.url = "http://server.lab83/papercraft/test/1/";
			this->summary_mirrored = false;

			MLIB_D("[%1] Mirroring item's '%2' (%3) summary...", this, this->item.title, this->item.url);

	#warning
	//disconnect(this->web_page, NULL, this, NULL);
	//this->web_page->deleteLater();
	//this->web_page = new QWebPage(this);
	//this->web_page->networkAccessManager()->setCache(this->cache);
	//connect(this->web_page, SIGNAL(loadFinished(bool)),
	//	this, SLOT(page_load_finished(bool)) );
			//this->web_page->mainFrame()->setHtml(item.summary);
			#warning
			//this->web_page->mainFrame()->setHtml(this->item.summary);
			//this->timeout_timer->start();
			emit this->page_load_finished(true);
		}
		catch(Storage::No_more_items&)
		{
			emit this->finished();
		}
		catch(Storage::No_selected_items&)
		{
			emit this->error(tr("Logical error."));
			return false;
		}
		catch(m::Exception& e)
		{
			emit this->error(EE(e));
			return false;
		}

		return true;
	}



	void Mirroring_stream::page_load_finished(bool ok)
	{
		MLIB_D("[%1] Page loading finished(%2).", this, ok);

		this->timeout_timer->stop();

		if(this->summary_mirrored)
			this->mirror_next();
		else
		{
			this->summary_mirrored = true;

			MLIB_D("[%1] Mirroring item's '%2' (%3) page...", this, this->item.title, this->item.url);

			this->web_page->mainFrame()->load(this->item.url);
			this->timeout_timer->start();
		}
	}



	void Mirroring_stream::page_loading_timed_out(void)
	{
		MLIB_D("[%1] Page loading timed out.", this);
		this->mirror_next();
	}


}



Download_feeds_items::Download_feeds_items(Storage* storage, QObject* parent)
:
	Task(parent),
	storage(storage)
{
	for(int i = 0; i < config::mirroring_streams_num; i++)
	{
		Mirroring_stream* stream = new Mirroring_stream(storage, this);

		connect(stream, SIGNAL(finished()),
			this, SLOT(stream_finished()) );

		connect(stream, SIGNAL(error(const QString&)),
			this, SLOT(stream_error(const QString&)) );

		this->streams << stream;
	}
}



void Download_feeds_items::process(void)
{
	MLIB_D("Downloading all feeds' items' content...");

#warning
#if 0 && GROV_OFFLINE_DEVELOPMENT
	this->finish();
	emit this->downloaded();
#else
	this->storage->set_current_source_to_all();

	// Running mirroring in all streams -->
	{
		QSet<Mirroring_stream*> streams = this->streams;
		Q_FOREACH(Mirroring_stream* stream, streams)
			if(!stream->mirror_next())
				break;
	}
	// Running mirroring in all streams <--
#endif
}



void Download_feeds_items::stream_error(const QString& error)
{
	Q_FOREACH(Mirroring_stream* stream, this->streams)
		stream->deleteLater();
	this->streams.clear();
	this->failed(error);
}



void Download_feeds_items::stream_finished(void)
{
	Mirroring_stream* stream =
		m::checked_qobject_cast<Mirroring_stream*>(this->sender());

	this->streams.remove(stream);
	stream->deleteLater();

	if(this->streams.empty())
	{
		this->finish();
		emit this->downloaded();
	}
}


}}}}

