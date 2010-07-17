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


#include <cstring>
#include <cstdlib>

#include <QtCore/QDir>
#include <QtCore/QProcess>
#include <QtCore/QTextCodec>

#include <QtGui/QApplication>

#include "messenger.hpp"

#include "core.hpp"


namespace m { namespace gui {


namespace {

	/// Current main window.
	QWidget* MAIN_WINDOW = NULL;

	/// Object that displays messages to the user.
	Messenger MESSENGER;


	/// Shows messages to the user.
	void	message_handler(const char* file, int line, m::Message_type type, const QString& title, const QString& message);



	void message_handler(const char* file, int line, m::Message_type type, const QString& title, const QString& message)
	{
		if(type == m::MESSAGE_TYPE_ERROR)
		{
			QStringList args;

			args << "--mlib-error-mode";
			args << file;
			args << QString::number(line);
			args << title;
			args << message;

			QProcess::startDetached(QDir(QCoreApplication::applicationFilePath()).absolutePath(), args);
			m::default_message_handler(file, line, type, title, message);
		}
		else
		{
			m::default_message_handler(file, line, type, title, message);
			MESSENGER.show(file, line, type, title, message);
		}
	}

}



QString format_window_title(const QString& title)
{
	if(title.isEmpty())
		return QCoreApplication::applicationName();
	else
	#if 0
		// May be in other applications
		return title + " - " + QCoreApplication::applicationName();
	#else
		return title;
	#endif
}



QWidget* get_main_window(void)
{
	return MAIN_WINDOW;
}



std::auto_ptr<QApplication> init(int& argc, char* argv[], const QString& app_name, Version app_version)
{
	bool error_mode = false;
	const char* file = "";
	int line = 0;
	const char* title = "";
	const char* error = "Unknown error.";

	// Checking for error displaying mode -->
		if(argc >= 2 && !std::strcmp(argv[1], "--mlib-error-mode"))
		{
			error_mode = true;

			if(argc == 6)
			{
				file = argv[2];
				line = atoi(argv[3]);
				title = argv[4];
				error = argv[5];
			}

			argc = 1;
			argv[argc] = NULL;
		}
	// Checking for error displaying mode <--

	// Configuring application -->
		std::auto_ptr<QApplication> app = std::auto_ptr<QApplication>(
			new QApplication(argc, argv) );

		QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());

		app->setApplicationName(app_name);
		app->setApplicationVersion(m::get_version_string(app_version));
	// Configuring application <--

	// Setting up message handlers -->
		m::set_message_handler(m::MESSAGE_TYPE_INFO, &message_handler);
	#if MLIB_DEVELOP_MODE
		m::set_message_handler(m::MESSAGE_TYPE_SILENT_WARNING, &message_handler);
	#endif
		m::set_message_handler(m::MESSAGE_TYPE_WARNING, &message_handler);
		m::set_message_handler(m::MESSAGE_TYPE_ERROR, &message_handler);
	// Setting up message handlers <--

	if(error_mode)
	{
		MESSENGER.show(file, line, m::MESSAGE_TYPE_ERROR, title, error);
		exit(EXIT_FAILURE);
	}
	else
		return app;
}



void set_main_window(QWidget* window)
{
	MAIN_WINDOW = window;
}


}}

