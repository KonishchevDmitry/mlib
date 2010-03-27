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

#include "task.hpp"


namespace grov { namespace client { namespace reader {


Task::Task(QObject* parent)
:
	QObject(parent),
	is_cancelled(false)
{
}



void Task::cancel(void)
{
	// TODO: realize
	MLIB_D("Task [%1] is cancelled.", this);
	this->is_cancelled = true;
}



bool Task::cancelled(void)
{
	return this->is_cancelled;
}


}}}

