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

#include <cstdlib>

/// GCC's attributes
#define MLIB_ATTRIBUTE(args...) __attribute__ (( args ))


namespace m { namespace messages_aux {


	/// See description of metaObject().
	class Meta_object
	{
		public:
			/// This function always returns empty name, but it helps special macroses to
			/// detail every debug message.
			QString	className(void) { return QString(); }
	};


	extern Debug_level DEBUG_LEVEL;
	extern Message_handler MESSAGE_HANDLERS[MESSAGE_TYPES_NUMBER];
	extern Meta_object FAKE_METAOBJECT;



	inline void	show_message_helper_no_return(const char* file, int line, Message_type type, const QString& message) MLIB_ATTRIBUTE(noreturn);
	inline void	show_message_helper_no_return(const char* file, int line, Message_type type, const QString& title, const QString& message) MLIB_ATTRIBUTE(noreturn);
	inline void	show_message_helper_return(const char* file, int line, Message_type type, const QString& message);
	inline void	show_message_helper_return(const char* file, int line, Message_type type, const QString& title, const QString& message);



	inline void show_message_helper_no_return(const char* file, int line, Message_type type, const QString& message)
	{
		(*MESSAGE_HANDLERS[type])(file, line, type, QString(), message);

		// To guarantee that error message terminate the program.
		abort();
	}



	inline void show_message_helper_no_return(const char* file, int line, Message_type type, const QString& title, const QString& message)
	{
		(*MESSAGE_HANDLERS[type])(file, line, type, title, message);

		// To guarantee that error message terminate the program.
		abort();
	}



	inline void show_message_helper_return(const char* file, int line, Message_type type, const QString& message)
	{
		(*MESSAGE_HANDLERS[type])(file, line, type, QString(), message);
	}



	inline void show_message_helper_return(const char* file, int line, Message_type type, const QString& title, const QString& message)
	{
		(*MESSAGE_HANDLERS[type])(file, line, type, title, message);
	}


}}


/// Some alternative to QObject::metaObject().
///
/// Every QObject has this method, and we always can get it's human-readable
/// name, so we can automatically detail every debug message, which function was
/// called from a QObject. But with other objects and functions we have no
/// ability to do this. So we provide an alternative (not so good, but still
/// usable).
inline m::messages_aux::Meta_object*	metaObject(void) { return &m::messages_aux::FAKE_METAOBJECT; };

