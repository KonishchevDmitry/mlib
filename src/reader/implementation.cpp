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


#include <QtCore/QVariant>

#include <QtNetwork/QNetworkCookie>
#include <QtNetwork/QNetworkRequest>

#include <src/common.hpp>
#include <src/reader/tasks/authenticate.hpp>
#include <src/reader/tasks/get_reading_list.hpp>
#include <src/storage.hpp>

#include "implementation.hpp"


namespace grov { namespace reader {


Implementation::Implementation(Storage* storage, const QString& user, const QString& password, QObject* parent)
:
	QObject(parent),

	storage(storage),

	user(user),
	password(password)
{
}



void Implementation::add_google_reader_task(Task_type type)
{
	// Even if we are already authenticated, the authentication SID could go
	// out of date. So we start authentication process every time we have no
	// pending tasks.
	if(this->pending_gr_tasks.empty())
	{
		tasks::Authenticate* task = new tasks::Authenticate(
			this->prepare_request(true), this->user, this->password );

		connect(task, SIGNAL(authenticated(const QString&)),
			this, SLOT(authenticated(const QString&)) );

		this->process_task(task);
	}

	this->pending_gr_tasks.enqueue(type);
}



void Implementation::authenticated(const QString& sid)
{
	MLIB_D("We authenticated at Google Reader with SID='%1'.", sid);
	this->sid = sid;

	const QNetworkRequest request = this->prepare_request(true);

	MLIB_D("Processing pending Google Reader's tasks...");
	while(!this->pending_gr_tasks.empty())
	{
		switch(this->pending_gr_tasks.dequeue())
		{
			case TASK_TYPE_GET_READING_LIST:
			{
				tasks::Get_reading_list* task = new tasks::Get_reading_list(request);

				connect(task, SIGNAL(items_gotten(const Feed_items_list&)),
					this, SLOT(items_gotten(const Feed_items_list&)) );

				connect(task, SIGNAL(reading_list_gotten()),
					this, SIGNAL(reading_list_gotten()) );

				this->process_task(task);
			}
			break;

			default:
				MLIB_LE();
				break;
		}
	}
}



void Implementation::get_reading_list(void)
{
	MLIB_D("Adding 'get reading list' task...");
	this->add_google_reader_task(TASK_TYPE_GET_READING_LIST);
}



void Implementation::items_gotten(const Feed_items_list& items)
{
	// TODO: exception
	this->storage->add_items(items);
}



QNetworkRequest Implementation::prepare_request(bool include_google_login)
{
	// TODO: other fields (User Agent, etc)

	QNetworkRequest request;

	if(include_google_login && !this->sid.isEmpty())
	{
		QNetworkCookie cookie("SID", sid.toAscii());
		cookie.setDomain(".google.com");
		cookie.setPath("/");

		QList<QNetworkCookie> cookies;
		cookies << cookie;
		request.setHeader(QNetworkRequest::CookieHeader, qVariantFromValue(cookies));
	}

	// TODO
	request.setRawHeader("User-Agent", "Mozilla/5.0 (X11; U; Linux x86_64; en-US; rv:1.9.1.8) Gecko/20100214 Ubuntu/9.10 (karmic) Firefox/3.5.8");

	return request;
}



void Implementation::process_task(Task* task)
{
	connect(task, SIGNAL(error(const QString&)),
		this, SLOT(task_error(const QString&)) );

	connect(this, SIGNAL(cancel_all_tasks()),
		task, SLOT(cancel()) );

	MLIB_D("Processing task [%1]...", task);
	task->process();
}



void Implementation::task_error(const QString& message)
{
	emit this->cancel_all_tasks();
	emit this->error(message);
}


}}

