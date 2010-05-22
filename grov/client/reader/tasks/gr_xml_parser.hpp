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


#ifndef GROV_HEADER_CLIENT_READER_TASKS_GR_XML_PARSER
#define GROV_HEADER_CLIENT_READER_TASKS_GR_XML_PARSER

class QDomDocument;

#include <QtCore/QHash>

#include <grov/common.hpp>
#include <grov/common/feed.hxx>
#include <grov/common/feed_item.hxx>


namespace grov { namespace client { namespace reader { namespace tasks {


/// Parses Google Reader's XML replies.
class Gr_xml_parser: public QObject
{
	Q_OBJECT

	public:
		/// Parses the reading list.
		///
		/// @param continuation_code - if not NULL, writes by pointer Google
		/// Reader continuation code or empty string, if there is no continuation
		/// code in \a data.
		///
		/// @throw m::Exception.
		Gr_feed_item_list		reading_list(const QByteArray& data, QString* continuation_code);

		/// Parses the stream preference list.
		///
		/// @return labels and states mapped to its subscription ordering.
		///
		/// @throw m::Exception.
		QHash<QString, QString>	stream_preference_list(const QByteArray& data);

		/// Parses the subscription list.
		///
		/// @throw m::Exception.
		Gr_feed_list			subscription_list(const QByteArray& data);

		/// Parses the tag list.
		///
		/// @return sort ids of all tags.
		///
		/// @throw m::Exception.
		QHash<QString, QString>	tag_list(const QByteArray& data);


	private:
		/// Creates QDomDocument fom \a data.
		///
		/// @throw m::Exception.
		static QDomDocument	get_dom(const QByteArray& data);
};


}}}}

#endif

