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

#include <cstdio>
#include <cstdlib>

#include <QtCore/QTextStream>
#include <QtCore/QTime>

#include <mlib/core/types.hpp>

#include "messages.hpp"


namespace m {

namespace
{


	const char* MESSAGES_PREFIXES[MESSAGE_TYPES_NUMBER] = {
		"D: ",
		"I: ",
		"W: ",
		"W: ",
		"E: "
	};



	void	debug_message_handler(const char* file, int line, const QString& title, const QString& message);
	void	info_message_handler(const char* file, int line, const QString& title, const QString& message);
	void	warning_message_handler(const char* file, int line, const QString& title, const QString& message);
	void	error_message_handler(const char* file, int line, const QString& title, const QString& message);



	void debug_message_handler(const char* file, int line, const QString& title, const QString& message)
	{
		QTextStream stream(stdout);
		print_message(stream, file, line, MESSAGE_TYPE_DEBUG, title, message);
	}



	void info_message_handler(const char* file, int line, const QString& title, const QString& message)
	{
		QTextStream stream(stdout);
		print_message(stream, file, line, MESSAGE_TYPE_INFO, title, message);
	}



	void warning_message_handler(const char* file, int line, const QString& title, const QString& message)
	{
		QTextStream stream(stderr);
		print_message(stream, file, line, MESSAGE_TYPE_WARNING, title, message);
	}



	void error_message_handler(const char* file, int line, const QString& title, const QString& message)
	{
		QTextStream stream(stderr);
		print_message(stream, file, line, MESSAGE_TYPE_ERROR, title, message);
	#if MLIB_DEVELOP_MODE
		std::abort();
	#else
		std::exit(EXIT_FAILURE);
	#endif
	}


}



namespace messages_aux
{
	Debug_level DEBUG_LEVEL = DEBUG_LEVEL_DISABLED;
	Meta_object FAKE_METAOBJECT;


	Message_handler MESSAGES_HANDLERS[MESSAGE_TYPES_NUMBER] = {
		&debug_message_handler,
		&info_message_handler,
		&warning_message_handler,
		&warning_message_handler,
		&error_message_handler
	};
}



Debug_level get_debug_level(void)
{
	return messages_aux::DEBUG_LEVEL;
}



void print_message(QTextStream& stream, const char* file, int line, Message_type type, QString title, const QString& message)
{
	if(title.isEmpty() && type == MESSAGE_TYPE_ERROR)
		title = QObject::tr("Application critical error");

	if(messages_aux::DEBUG_LEVEL >= DEBUG_LEVEL_ENABLED)
	{
		const size_t file_path_len = 15;

		stream
			<< '(' << QTime::currentTime().toString("HH:mm:ss.zzz") << ") ("
			<< QString(file).right(file_path_len).rightJustified(file_path_len)
			<< ':' << qSetFieldWidth(4) << qSetPadChar('0') << line << qSetFieldWidth(0) << ") ";
	}

	stream << MESSAGES_PREFIXES[type];

	if(!title.isEmpty())
		stream << '[' << title << "] ";

	stream << message << endl;
}



void set_debug_level(Debug_level level)
{
	messages_aux::DEBUG_LEVEL = level;
}



void set_messages_handler(Message_type type, Message_handler handler)
{
	messages_aux::MESSAGES_HANDLERS[type] = handler;
}


}

