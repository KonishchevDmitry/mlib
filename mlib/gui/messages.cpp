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


#include <QtCore/QCoreApplication>

#include <QtGui/QMessageBox>
#include <QtGui/QTextDocument>

#include "messages.hpp"



namespace m { namespace gui {


void show_message(QWidget* parent, Message_type type, QString title, const QString& message, const QString& details)
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

	QString window_title = QCoreApplication::applicationName();
	if(!title.isEmpty())
		window_title.prepend(title + " - ");

	QString message_box_title = "<b>" + Qt::escape(title) + "</b> ";

	// TODO: dirty hack
	if(message_box_title.size() < 40)
	{
		message_box_title.replace(' ', "&nbsp;");

		for(size_t i = 0; i < 40; i++)
			message_box_title += "&nbsp;";
	}

	QMessageBox message_box(icon, window_title, message, QMessageBox::Ok, parent);

	message_box.setTextFormat(Qt::RichText);
	message_box.setText(message_box_title);
	message_box.setInformativeText(Qt::escape(message));

	if(!details.isEmpty())
		message_box.setDetailedText(Qt::escape(details));

	message_box.setDefaultButton(QMessageBox::Ok);
	message_box.exec();
}



void show_warning_message(QWidget* parent, const QString& title, const QString& message, const QString& details)
{
	show_message(parent, MESSAGE_TYPE_WARNING, title, message, details);
}



bool yes_no_message(QWidget* parent, const QString& title, const QString& message)
{
	QString window_title = title + " - " + QCoreApplication::applicationName();

	QMessageBox message_box(QMessageBox::Question, window_title, message, QMessageBox::Yes | QMessageBox::No, parent);

	message_box.setTextFormat(Qt::RichText);
	message_box.setText("<b>" + Qt::escape(title) + "</b>");
	message_box.setInformativeText(Qt::escape(message));
	message_box.setDefaultButton(QMessageBox::No);
	return message_box.exec() == QMessageBox::Yes;
}


}}

