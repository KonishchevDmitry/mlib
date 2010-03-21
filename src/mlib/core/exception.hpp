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
#ifndef MLIB_HEADER_CORE_EXCEPTION
#define MLIB_HEADER_CORE_EXCEPTION

#include <exception>

#include <QString>

#include <mlib/core/format.hpp>


#define M_THROW(args...) throw m::Exception(__FILE__, __LINE__, _F(args))

namespace m {

class Exception: public std::exception
{
	public:
		Exception(const char* file, int line, const QString& error);
		virtual ~Exception(void) throw() {}


	private:
		QString	error;


	public:
		virtual const char*	what(void) throw();

		/// Returns an error string;
		const QString&		string(void) const;
};

}

// TODO
inline QString EE(const m::Exception& e) { return e.string(); }

#endif

