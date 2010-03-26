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

// TODO: rewrite
#include <mlib/core/messages.hpp>

#include "exception.hpp"

namespace m {

	Exception::Exception(const char* file, int line, const QString& error)
	:
		error(error)
	{
		QString error_string = _F("Exception: %1", error);
		// TODO: make as MLIB_DV
		_MLIB_SHOW_MESSAGE_FROM(file, line, MESSAGE_TYPE_DEBUG, return, error_string);
	}



	const char* Exception::what(void) throw()
	{
	// TODO
		MLIB_E("error");
		return NULL;
	}



	const QString& Exception::string(void) const
	{
		return this->error;
	}

}

