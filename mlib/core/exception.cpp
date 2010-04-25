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


#include <QtCore/QtGlobal>

#ifdef Q_OS_UNIX
	#include <cerrno>
	#include <cstring>
#endif

#include <mlib/core/messages.hpp>

#include "exception.hpp"


namespace m {


Exception::Exception(const char* file, int line, const QString& error)
:
	error(error)
{
#if MLIB_DEBUG_MODE
	if(messages_aux::DEBUG_LEVEL >= DEBUG_LEVEL_ENABLED)
		_MLIB_SHOW_MESSAGE_FROM(file, line, MESSAGE_TYPE_DEBUG, return, "Exception", error);
#endif
}



#ifdef Q_OS_UNIX
	QString EE(void)
	{
		return strerror(errno);
	}
#endif


}

