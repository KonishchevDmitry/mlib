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


#ifndef MLIB_HEADER_GUI_MESSAGES
#define MLIB_HEADER_GUI_MESSAGES

class QWidget;

#include <mlib/core.hpp>


namespace m { namespace gui {


/// Shows GUI message, blocking until the user closes it.
void	show_message(QWidget* parent, Message_type type, QString title, const QString& message, const QString& details = QString(), bool block = true);

/// Shows warning message, blocking until the user closes it.
void	show_warning_message(QWidget* parent, const QString& title, const QString& message, const QString& details = QString(), bool block = true);

/// Shows "Yes or No" message.
///
/// Returns true, if user answer yes.
bool	yes_no_message(QWidget* parent, const QString& title, const QString& message);


}}

#endif

