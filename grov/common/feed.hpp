/**************************************************************************
*                                                                         *
*   Grov - Google Reader offline viewer                                   *
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


#ifndef GROV_HEADER_COMMON_FEED
#define GROV_HEADER_COMMON_FEED

#include <QtCore/QSet>

#include <grov/common.hpp>

#include "feed.hxx"


namespace grov {


/// Represents a RSS feed gotten from Google Reader's subscription list.
class Gr_feed
{
	public:
		/// Feed's Google Reader id.
		QString			gr_id;

		/// Feed name.
		QString			name;

		/// Feed's labels.
		QSet<QString>	labels;

		/// Feed's Google Reader sort id.
		QString 		sort_id;
};


}

#endif

