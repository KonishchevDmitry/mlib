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
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
**************************************************************************/


#include "core.hpp"
#include "message_box.hpp"

#include "messages.hpp"


namespace m { namespace gui {


void show_message(QWidget* parent, Message_type type, QString title, const QString& message, const QString& details, bool block)
{
	QMessageBox::Icon icon;

	switch(type)
	{
		case MESSAGE_TYPE_INFO:
			icon = QMessageBox::Information;
			if(title.isEmpty())
				title = QObject::tr("Information");
			break;

		case MESSAGE_TYPE_SILENT_WARNING:
		case MESSAGE_TYPE_WARNING:
			icon = QMessageBox::Warning;
			if(title.isEmpty())
				title = QObject::tr("Warning");
			break;

		case MESSAGE_TYPE_ERROR:
			icon = QMessageBox::Critical;
			if(title.isEmpty())
				title = QObject::tr("Application critical error");
			break;

		default:
			icon = QMessageBox::NoIcon;
			break;
	}

	Message_box* message_box = new Message_box(icon, title, message, QMessageBox::Ok, parent);

	if(!details.isEmpty())
		message_box->set_details(details);

	message_box->setDefaultButton(QMessageBox::Ok);

	if(block)
	{
		message_box->exec();
		delete message_box;
	}
	else
	{
		message_box->connect(message_box, SIGNAL(finished(int)), SLOT(deleteLater()));
		message_box->show();
	}
}



void show_warning_message(QWidget* parent, const QString& title, const QString& message, const QString& details, bool block)
{
	show_message(parent, MESSAGE_TYPE_WARNING, title, message, details, block);
}



bool yes_no_message(QWidget* parent, const QString& title, const QString& message)
{
	Message_box message_box(
		QMessageBox::Question, title, message,
		QMessageBox::Yes | QMessageBox::No, parent
	);
	message_box.setDefaultButton(QMessageBox::No);
	return message_box.exec() == QMessageBox::Yes;
}


}}

