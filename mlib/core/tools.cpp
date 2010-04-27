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


#include "tools.hpp"


namespace m {


Version get_major_version(Version version)
{
	return version / 100 / 100;
}



Version get_minor_version(Version version)
{
	return version % ( 100 * 100 ) / 100;
}



Version get_patch_version(Version version)
{
	return version % 100;
}



Version get_version(Version major, Version minor, Version patch)
{
	return MLIB_GET_VERSION(major, minor, patch);
}



QString get_version_string(Version version)
{
	Version patch_version = get_patch_version(version);

	return
		QString::number(get_major_version(version)) +
		"." + QString::number(get_minor_version(version)) +
		( patch_version ? "." + QString::number(patch_version) : QString() );
}


}

