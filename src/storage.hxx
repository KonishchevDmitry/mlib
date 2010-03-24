/**************************************************************************
*                                                                         *
*   grov - Google Reader offline viewer                                   *
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


#ifndef GROV_HEADER_STORAGE_FWD
#define GROV_HEADER_STORAGE_FWD

#include <QtCore/QPair>

#include <src/common.hpp>


namespace grov
{
	#warning
	#if 0
	/// Feed's id and name.
	typedef QPair<Big_id, QString> Feed_info;

	/// Label's id and name.
	typedef QPair<Big_id, QString> Label_info;

	typedef QList<Feed_info> Feeds_info;

	/// Stores feeds that belongs to label.
	typedef QPair<Label_info, Feeds_info> Label_to_feeds_map;

	/// Represents a feeds tree as a list of labels associated with feeds.
	///
	/// Feeds which has not any label, belongs to id == Storage::NO_LABEL_ID.
	typedef QList<Label_to_feeds_map> Feeds_tree;
	#endif

	class Storage;
}

#endif

