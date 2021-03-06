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


#ifndef MLIB_HEADER_GUI_CORE
#define MLIB_HEADER_GUI_CORE

#include <memory>

class QApplication;

#include <QtGui/QWidget>

#include <mlib/core.hpp>


namespace m { namespace gui {

/// Formats window title to the form "$title - $app_name".
QString						format_window_title(const QString& title);

/// Initializes a GUI application.
std::auto_ptr<QApplication> init(int& argc, char* argv[], const QString& app_name, Version app_version);

/// Returns the main window or NULL if it had not been setted yet.
QWidget*					get_main_window(void);

/// Sets current main window (some GUI tools needs it for e.g. to display
/// messages on top of it).
void						set_main_window(QWidget* window);

}}

#endif

