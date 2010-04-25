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


#include <QtGui/QTextDocument>

#include "core.hpp"

#include "message_box.hpp"


namespace m { namespace gui {


Message_box::Message_box(Icon icon, const QString& title, const QString& message, StandardButtons buttons, QWidget* parent, Qt::WindowFlags f)
:
	QMessageBox(icon, m::gui::format_window_title(title), "<b>" + Qt::escape(title) + "</b>", buttons, parent, f)
{
	this->setTextFormat(Qt::RichText);
	this->set_message(message);
}



int Message_box::exec(void)
{
	this->show();
	return QMessageBox::exec();
}



void Message_box::set_details(const QString& details)
{
	// If message does not have '<' or '>' symbols, Qt thinks about it as
	// of plain text message and shows to user an "&amp;" instead of a '&',
	// etc.
	this->setDetailedText("<b></b>" + Qt::escape(details));
}



void Message_box::set_message(const QString& message)
{
	// If message does not have '<' or '>' symbols, Qt thinks about it as
	// of plain text message and shows to user an "&amp;" instead of a '&',
	// etc.
	this->setInformativeText("<b></b>" + Qt::escape(message));
}



void Message_box::set_title(const QString& title)
{
	this->setWindowTitle(m::gui::format_window_title(title));
	this->setText("<b>" + Qt::escape(title) + "</b>");
}



void Message_box::show(void)
{
	QMessageBox::show();

	// QMessageBox::show() resets widget's minimumWidth(), so we manually show
	// the dialog and set its minimum width.
	this->setMinimumWidth(300);
}


}}

