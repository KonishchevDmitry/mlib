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


Download_feeds_items::Download_feeds_items(Storage* storage, QObject* parent)
:
	Task(parent),
	storage(storage),
	cache(new Web_cache(storage, this)),
	web_page(new QWebPage(this)),
	timeout_timer(new QTimer(this))
{
	this->web_page->networkAccessManager()->setCache(this->cache);

	this->timeout_timer->setSingleShot(true);
	this->timeout_timer->setInterval(config::page_mirroring_timeout * 1000);
	connect(this->timeout_timer, SIGNAL(timeout()),
		this, SLOT(page_loading_timed_out()) );

	connect(this->web_page, SIGNAL(loadFinished(bool)),
		this, SLOT(page_load_finished(bool)) );
}



void Download_feeds_items::cancel(void)
{
	disconnect(this->timeout_timer, NULL, this, NULL);
	disconnect(this->web_page, NULL, this, NULL);
	this->storage->set_current_source_to_none();
}



void Download_feeds_items::mirror_next(void)
{
	try
	{
		Db_feed_item item = this->storage->get_next_item();

		// TODO: url
		MLIB_D("Mirroring item's '%1' summary...", item.title);

		this->web_page->mainFrame()->setHtml(item.summary);
		this->timeout_timer->start();
	}
	catch(Storage::No_more_items&)
	{
		this->storage->set_current_source_to_none();
		emit this->downloaded();
		this->finish();
	}
	catch(Storage::No_selected_items&)
	{
		this->failed(tr("Logical error."));
	}
	catch(m::Exception& e)
	{
		this->failed(EE(e));
	}
}



void Download_feeds_items::page_load_finished(bool ok)
{
	MLIB_D("Page loading finished(%1).", ok);
	this->timeout_timer->stop();
	this->mirror_next();
}



void Download_feeds_items::page_loading_timed_out(void)
{
	MLIB_D("Page loading timed out.");
	this->mirror_next();
}



void Download_feeds_items::process(void)
{
	MLIB_D("Downloading all feeds' items' content...");

#if GROV_OFFLINE_DEVELOPMENT
	emit this->downloaded();
	this->finish();
#else
	this->storage->set_current_source_to_all();
	this->mirror_next();
#endif
}


}}}}

