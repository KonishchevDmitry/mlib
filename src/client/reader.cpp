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


#include <QtNetwork/QNetworkCookieJar>

#include <src/common.hpp>

#include "reader/tasks/authenticate.hpp"
#include "reader/tasks/get_reading_list.hpp"

#include "reader.hpp"


namespace grov { namespace client {


Reader::Reader(Storage* storage, const QString& user, const QString& password, QObject* parent)
:
	QObject(parent),

	storage(storage),
	cookies(new QNetworkCookieJar(this)),

	user(user),
	password(password)
{
}



void Reader::add_google_reader_task(Task_type type)
{
	bool no_tasks = this->pending_gr_tasks.empty();

	this->pending_gr_tasks.enqueue(type);

	// Even if we are already authenticated, the authentication could go out of
	// date. So we start authentication process every time we have no pending
	// tasks.
	if(no_tasks)
	{
		reader::tasks::Authenticate* task = new reader::tasks::Authenticate(
			this, this->user, this->password, this );

		connect(task, SIGNAL(authenticated(const QString&)),
			this, SLOT(authenticated(const QString&)) );

		this->process_task(task);
	}
}



void Reader::authenticated(const QString& auth_id)
{
	MLIB_D("We authenticated at Google Reader with Auth=%1.", auth_id);
	this->auth_id = auth_id;

	MLIB_D("Processing pending Google Reader's tasks...");
	while(!this->pending_gr_tasks.empty())
	{
		switch(this->pending_gr_tasks.dequeue())
		{
			case TASK_TYPE_GET_READING_LIST:
			{
				reader::tasks::Get_reading_list* task =
					new reader::tasks::Get_reading_list(this, this);

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



void Reader::get_reading_list(void)
{
	MLIB_D("Adding 'get reading list' task...");
	this->add_google_reader_task(TASK_TYPE_GET_READING_LIST);
}



//void Reader::items_gotten(const Feed_items_list& items)
//{
//	// TODO: exception
//	this->storage->add_items(items);
//}



void Reader::process_task(reader::Task* task)
{
	connect(task, SIGNAL(error(const QString&)),
		this, SLOT(task_error(const QString&)) );

	connect(this, SIGNAL(cancel_all_tasks()),
		task, SLOT(cancel()) );

	MLIB_D("Processing task [%1]...", task);
	task->process();
}



void Reader::task_error(const QString& message)
{
	emit this->cancel_all_tasks();
	emit this->error(message);
}


}}

