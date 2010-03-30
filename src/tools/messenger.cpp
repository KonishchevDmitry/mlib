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
#include <QtGui/QMessageBox>

#include <src/common.hpp>
#include <src/main.hpp>

#include "messenger.hpp"


namespace grov { namespace tools {


Messenger::Messenger(QObject* parent)
:
	QObject(parent)
{
	connect(this, SIGNAL(message(const char*, int, m::Message_type, QString, const QString&)),
		this, SLOT(on_message(const char*, int, m::Message_type, QString, const QString&)) );
}



void Messenger::show(const char* file, int line, m::Message_type type, const QString& title, const QString& message)
{
	emit this->message(file, line, type, title, message);
}



void Messenger::on_message(const char* file, int line, m::Message_type type, QString title, const QString& message)
{
	QMessageBox::Icon icon;

	switch(type)
	{
		case m::MESSAGE_TYPE_INFO:
			icon = QMessageBox::Information;
			break;

		case m::MESSAGE_TYPE_WARNING:
			icon = QMessageBox::Warning;
			break;

		case m::MESSAGE_TYPE_ERROR:
			icon = QMessageBox::Critical;

			if(title.isEmpty())
				title = QObject::tr("Application critical error");

			break;

		default:
			icon = QMessageBox::NoIcon;
			break;
	}

	QString window_title = GROV_APP_NAME;
	if(!title.isEmpty())
		window_title.prepend(title + " - ");

	QMessageBox message_box(icon, window_title, message, QMessageBox::Ok, get_main_window());

	message_box.setTextFormat(Qt::RichText);
	// TODO: remove tags from string
	message_box.setText("<b>" + title + "</b>");
	// TODO: remove tags from string
	message_box.setInformativeText(message);

	if(type == m::MESSAGE_TYPE_ERROR)
	{
		QString debug;

		debug += _F("%1 %2\n\n", GROV_APP_NAME, get_version());
		debug += _F( tr("Error happened at %1:%2. Please contact to developer."), file, line );

		// TODO: remove tags from string
		message_box.setDetailedText(debug);
	}

	message_box.setDefaultButton(QMessageBox::Ok);
	message_box.exec();
}


}}

