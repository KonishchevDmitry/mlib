/**************************************************************************
*                                                                         *
*   MLib - library of some useful things for internal usage               *
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


#include <QtCore/QCoreApplication>

#include "core.hpp"
#include "messages.hpp"

#include "messenger.hpp"


namespace m { namespace gui {


Messenger::Messenger(QObject* parent)
:
	QObject(parent)
{
	connect(this, SIGNAL(message(const QString&, int, m::Message_type, const QString&, const QString&)),
		this, SLOT(on_message(const QString&, int, m::Message_type, const QString&, const QString&)) );
}



void Messenger::show(const QString& file, int line, m::Message_type type, const QString& title, const QString& message)
{
	emit this->message(file, line, type, title, message);
}



void Messenger::on_message(const QString& file, int line, m::Message_type type, const QString& title, const QString& message)
{
	QString details;

	if(type == m::MESSAGE_TYPE_ERROR)
	{
		details += _F("%1 %2\n\n", QCoreApplication::applicationName(), QCoreApplication::applicationVersion());
		details += _F( tr("Error happened at %1:%2. Please contact to developer."), file, line );
	}

	m::gui::show_message(
		get_main_window(), type,
		title, message, details,
		type != m::MESSAGE_TYPE_SILENT_WARNING
	);
}


}}

