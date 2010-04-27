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


#ifndef MLIB_HEADER_GUI_MESSAGE_BOX
#define MLIB_HEADER_GUI_MESSAGE_BOX

#include <QtGui/QMessageBox>

#include "core.hpp"

#include "message_box.hxx"


namespace m { namespace gui {


/// Fixes few bugs in QMessageBox and adds extra functionality.
class Message_box: public QMessageBox
{
	Q_OBJECT

	public:
		Message_box(Icon icon, const QString& title, const QString& message, StandardButtons buttons = NoButton, QWidget* parent = 0, Qt::WindowFlags f = Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);


	public:
		/// Sets message box's details text.
		void	set_details(const QString& details);

		/// Sets message box's message text.
		void	set_message(const QString& message);

		/// Sets message box's title.
		void	set_title(const QString& title);
};


}}

#endif

