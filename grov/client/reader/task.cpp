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


#include <grov/common.hpp>

#include "task.hpp"


namespace grov { namespace client { namespace reader {


Task::Task(QObject* parent)
:
	QObject(parent),
	finished(false)
{
	MLIB_D("Task [%1] created.", this);
}



Task::~Task(void)
{
	if(this->is_cancelled())
	{
		MLIB_D("Task [%1] is cancelled.", this);
		emit this->cancelled();
	}

	MLIB_D("Task [%1] destroyed.", this);
}



void Task::cancel(void)
{
	this->deleteLater();
}



void Task::child_task_error(const QString& message)
{
	MLIB_D("Task's [%1] child task emitted error '%2'.", this, message);
	this->failed(message);
}



void Task::failed(const QString& message)
{
	this->finish();
	emit this->error(message);
}



void Task::finish(void)
{
	MLIB_D("Finalizing task [%1]...", this);
	this->finished = true;
	this->deleteLater();
}



bool Task::is_cancelled(void)
{
	return !this->finished;
}



void Task::process_task(Task* task)
{
	connect(task, SIGNAL(error(const QString&)),
		this, SLOT(child_task_error(const QString&)), Qt::QueuedConnection );

	task->process();
}


}}}

