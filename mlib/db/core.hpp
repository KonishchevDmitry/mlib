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


#ifndef MLIB_HEADER_DB_CORE
#define MLIB_HEADER_DB_CORE

class QSqlDatabase;
class QSqlError;
class QSqlQuery;

#include <mlib/core.hpp>


namespace m {


// Returns text of database's last error.
QString	EE(const QSqlDatabase& db);

// Returns database error's string.
QString	EE(const QSqlError& error);

// Returns text of query's last error.
QString	EE(const QSqlQuery& query);


}

#if MLIB_ENABLE_ALIASES
	using m::EE;
#endif

#endif

