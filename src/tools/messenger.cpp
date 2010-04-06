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
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
**************************************************************************/


#include <QtGui/QMainWindow>

#include <src/common.hpp>
#include <src/main.hpp>

#include <mlib/gui/messages.hpp>

#include "messenger.hpp"


namespace grov { namespace tools {


Messenger::Messenger(QObject* parent)
:
	QObject(parent)
{
	connect(this, SIGNAL(message(const char*, int, m::Message_type, const QString&, const QString&)),
		this, SLOT(on_message(const char*, int, m::Message_type, const QString&, const QString&)) );
}



void Messenger::show(const char* file, int line, m::Message_type type, const QString& title, const QString& message)
{
	emit this->message(file, line, type, title, message);
}



void Messenger::on_message(const char* file, int line, m::Message_type type, const QString& title, const QString& message)
{
	QString details;

	if(type == m::MESSAGE_TYPE_ERROR)
	{
		details += _F("%1 %2\n\n", GROV_APP_NAME, get_version());
		details += _F( tr("Error happened at %1:%2. Please contact to developer."), file, line );
	}

	m::gui::show_message(get_main_window(), type, title, message, details);
}


}}

