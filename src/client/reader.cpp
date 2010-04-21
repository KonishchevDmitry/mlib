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


#include <src/common.hpp>

#include "reader/tasks/flush_offline_data.hpp"
#include "reader/tasks/get_reading_list.hpp"

#include "reader.hpp"


namespace grov { namespace client {


Reader::Reader(Storage* storage, QObject* parent)
:
	QObject(parent),
	storage(storage)
{
}



void Reader::cancel(void)
{
	MLIB_D("Cancelling all current tasks...");
	emit this->cancel_all_tasks();
}



void Reader::flush_offline_data(const QString& login, const QString& password)
{
	MLIB_D("Starting 'flush offline data' task...");

	reader::tasks::Flush_offline_data* task =
		new reader::tasks::Flush_offline_data(this->storage, login, password, this);

	connect(task, SIGNAL(flushed()),
		this, SIGNAL(offline_data_flushed()), Qt::QueuedConnection );

	this->process_task(task);
}



void Reader::get_reading_list(const QString& login, const QString& password)
{
	MLIB_D("Starting 'get reading list' task...");

	reader::tasks::Get_reading_list* task =
		new reader::tasks::Get_reading_list(this->storage, login, password, this);

	connect(task, SIGNAL(reading_list_gotten()),
		this, SIGNAL(reading_list_gotten()), Qt::QueuedConnection );

	this->process_task(task);
}



void Reader::process_task(reader::Task* task)
{
	connect(this, SIGNAL(cancel_all_tasks()),
		task, SLOT(cancel()) );

	connect(task, SIGNAL(cancelled()),
		this, SIGNAL(cancelled()), Qt::QueuedConnection );

	connect(task, SIGNAL(error(const QString&)),
		this, SIGNAL(error(const QString&)), Qt::QueuedConnection );

	task->process();
}


}}

