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
typedef qint64 Big_id;

/// For file's and other sizes.
typedef qint64 Size;

/// For time in seconds.
typedef qint64 Time;

/// For time in milliseconds
typedef qint64 Time_ms;

/// Major, minor and patch version encoded to one integer.
typedef qint32 Version;


/// Converts QVariant to Big_id.
Big_id	qvariant_to_big_id(const QVariant& variant);

/// Converts QVariant to Version.
Version	qvariant_to_version(const QVariant& variant);


}

#if MLIB_ENABLE_ALIASES
	using m::Big_id;
	using m::Size;
	using m::Time;
	using m::Time_ms;
	using m::Version;
#endif

#endif

