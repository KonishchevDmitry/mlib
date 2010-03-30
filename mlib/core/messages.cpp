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


namespace {

	const char* MESSAGES_PREFIXES[MESSAGE_TYPES_NUMBER] = {
		"D: ",
		"I: ",
		"W: ",
		"W: ",
		"E: "
	};

}


namespace messages_aux
{
	Debug_level DEBUG_LEVEL = DEBUG_LEVEL_DISABLED;
	Meta_object FAKE_METAOBJECT;


	Message_handler MESSAGE_HANDLERS[MESSAGE_TYPES_NUMBER] = {
		&default_message_handler,
		&default_message_handler,
		&default_message_handler,
		&default_message_handler,
		&default_message_handler
	};
}



void default_message_handler(const char* file, int line, Message_type type, const QString& title, const QString& message)
{
	switch(type)
	{
		case MESSAGE_TYPE_DEBUG:
		case MESSAGE_TYPE_INFO:
		{
			QTextStream stream(stdout);
			print_message(stream, file, line, type, title, message);
		}
		break;

		case MESSAGE_TYPE_SILENT_WARNING:
		case MESSAGE_TYPE_WARNING:
		{
			QTextStream stream(stderr);
			print_message(stream, file, line, type, title, message);
		}
		break;

		default:
		{
			QTextStream stream(stderr);
			print_message(stream, file, line, type, title, message);
		#if MLIB_DEVELOP_MODE
			std::abort();
		#else
			std::exit(EXIT_FAILURE);
		#endif
		}
		break;
	}
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
		stream << title << ". ";

	stream << message << endl;
}



void set_debug_level(Debug_level level)
{
	messages_aux::DEBUG_LEVEL = level;
}



void set_message_handler(Message_type type, Message_handler handler)
{
	messages_aux::MESSAGE_HANDLERS[type] = handler;
}


}

