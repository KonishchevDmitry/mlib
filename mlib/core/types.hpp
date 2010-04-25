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


#ifndef MLIB_HEADER_CORE_TYPES
#define MLIB_HEADER_CORE_TYPES

#include <string>

class QVariant;

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QStringList>


namespace m {


/// Id for databases' entries and for other storages with big amount of data.
typedef long long Big_id;

/// For file's and other sizes.
typedef long long Size;


/// Converts QVariant to Big_id.
Big_id	qvariant_to_big_id(const QVariant& variant);


}

#if MLIB_ENABLE_ALIASES
	using m::Big_id;
	using m::Size;
#endif

#endif

