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


#ifndef MLIB_HEADER_CORE_EXCEPTION
#define MLIB_HEADER_CORE_EXCEPTION

#include <QtCore/QString>
#include <QtCore/QtGlobal>

#include <mlib/core/format.hpp>


#define M_THROW(args...) throw m::Exception(__FILE__, __LINE__, _F(args))


namespace m {


/// Convenient for using and debugging exception class.
class Exception
{
	public:
		Exception(const char* file, int line, const QString& error);


	private:
		/// Error string.
		QString	error;


	public:
		/// Returns an error string;
		inline const QString&	string(void) const;
};



#ifdef Q_OS_UNIX
/// Returns strerror(errno) string.
QString					EE(void);
#endif

// Same as std::exception::what(), but returns a QString and may be formats an
// exception message.
inline const QString&	EE(const m::Exception& e);


}


#include "exception.hh"

#if MLIB_ENABLE_ALIASES
	using m::EE;
#endif

#endif

