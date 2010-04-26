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

#ifndef MLIB_HEADER_CORE_MESSAGES
#define MLIB_HEADER_CORE_MESSAGES

class QTextStream;

#include <mlib/core/format.hpp>
#include <mlib/core/types.hpp>


#define _MLIB_SHOW_MESSAGE_FROM(file, line, type, return_spec, args...) ::m::messages_aux::show_message_helper_ ## return_spec ( (file), (line), (type), args )
#define _MLIB_SHOW_MESSAGE(type, return_spec, args...) _MLIB_SHOW_MESSAGE_FROM(__FILE__, __LINE__, (type), return_spec, args)

#if MLIB_DEBUG_MODE
	#define _MLIB_DEBUG_CONTEXT() \
	({ \
		QString context = metaObject()->className(); \
		\
		if(context.isEmpty()) \
			context = __FUNCTION__; \
		\
		context; \
	})

	/// To avoid message formatting when debug is disabled.
	#define _MLIB_SHOW_DEBUG_MESSAGE(level, return_spec, message) \
	({ \
		if(::m::messages_aux::DEBUG_LEVEL >= (level)) \
			_MLIB_SHOW_MESSAGE( ::m::MESSAGE_TYPE_DEBUG, return_spec, _MLIB_DEBUG_CONTEXT(), (message) ); \
	})
#endif

/// Needs to be placed in every static class member in which you use MLib's
/// debug functions.
#define MLIB_STATIC_METHOD_DEBUG_WORKAROUND using ::metaObject;

/// Shows a debug message.
///
/// @param args - the same arguments as in m::_F().
#if MLIB_DEBUG_MODE
	#define MLIB_D(args...) _MLIB_SHOW_DEBUG_MESSAGE(::m::DEBUG_LEVEL_ENABLED, return, _F(args))
#else
	#define MLIB_D(args...)
#endif

/// Shows a verbose debug message.
///
/// @param args - the same arguments as in m::_F().
#if MLIB_DEBUG_MODE
	#define MLIB_DV(args...) _MLIB_SHOW_DEBUG_MESSAGE(::m::DEBUG_LEVEL_VERBOSE, return, _F(args))
#else
	#define MLIB_DV(args...)
#endif

/// Shows an info message.
///
/// @param title - warning title message
/// @param args - the same arguments as in m::_F().
#define MLIB_I(args...) _MLIB_SHOW_MESSAGE(::m::MESSAGE_TYPE_INFO, return, args)

/// Shows a warning message.
///
/// @param title - warning title message
/// @param args - the same arguments as in m::_F().
#define MLIB_W(args...) _MLIB_SHOW_MESSAGE(::m::MESSAGE_TYPE_WARNING, return, args)

/// Shows a silent warning message.
///
/// @param title (QString) - message title (optional).
/// @param message (QString) - message to show.
#define MLIB_SW(args...) _MLIB_SHOW_MESSAGE(::m::MESSAGE_TYPE_SILENT_WARNING, return, args)

/// Shows a silent warning message in the develop mode and a debug message in
/// the other case.
///
/// @param args - the same arguments as in m::_F().
#if MLIB_DEVELOP_MODE
	#define MLIB_DW(args...) MLIB_SW("", _F(args))
#else
	#define MLIB_DW(args...) MLIB_D(args)
#endif

/// Shows an error message.
///
/// @param title (QString) - message title (optional).
/// @param message (QString) - message to show.
#define MLIB_E(args...) _MLIB_SHOW_MESSAGE(::m::MESSAGE_TYPE_ERROR, no_return, args)

/// Logical error.
#define MLIB_LE() MLIB_E(QObject::tr("Logical error."))

/// Assert.
#define MLIB_A(assertion, args...) ({ if(!(assertion)) MLIB_LE(); })


namespace m {


/// Message type.
///
/// \attention
/// (( For the library's developers ))
/// If you change the types order you must adjust this changes to
/// MESSAGES_* arrays.
enum Message_type {
	/// Debug message.
	MESSAGE_TYPE_DEBUG,

	/// Info message (to terminal or GUI).
	MESSAGE_TYPE_INFO,

	/// Warning message that is not boring for users (to terminal).
	MESSAGE_TYPE_SILENT_WARNING,

	/// Warning message (to terminal or GUI).
	MESSAGE_TYPE_WARNING,

	/// Error message (to terminal or GUI with following program
	/// termination).
	MESSAGE_TYPE_ERROR,


	/// Number of existing message types.
	MESSAGE_TYPES_NUMBER
};


/// Determines which type of debug messages will be displayed.
enum Debug_level
{
	/// No debug messages will be displayed.
	DEBUG_LEVEL_DISABLED,

	/// All MLIB_D() messages will be displayed.
	DEBUG_LEVEL_ENABLED,

	/// All MLIB_D() and MLIB_DV() messages will be displayed.
	DEBUG_LEVEL_VERBOSE
};


/// Type of custom function for message printing.
typedef void (*Message_handler)(const char*, int, Message_type type, const QString&, const QString&);



/// MLib's default message handler.
void		default_message_handler(const char* file, int line, Message_type type, const QString& title, const QString& message);

/// Returns current debug level.
Debug_level	get_debug_level(void);

/// Prints a message.
void		print_message(QTextStream& stream, const char* file, int line, Message_type type, QString title, const QString& message);

/// Sets the debug level.
void		set_debug_level(Debug_level level);

/// Sets message handler.
///
/// \attention
/// Error message handler must terminate the program.
void		set_message_handler(Message_type type, Message_handler handler);


}


#include "messages.hh"

#endif

