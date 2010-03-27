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


#include <QtCore/QSet>
#include <QtXml/QDomDocument>

#include <src/common.hpp>
#include <src/common/feed_item.hpp>

#include "items_list_parser.hpp"


namespace grov { namespace client { namespace reader { namespace tasks {


QList<Feed_item> Items_list_parser::parse(const QByteArray& data)
{
	MLIB_DV("Parsing feeds' items list:");

	QDomDocument xml;

	// Building a DOM tree -->
	{
		QString error;
		int error_line;
		int error_column;

		if(!xml.setContent(data, false, &error, &error_line, &error_column))
			M_THROW(CSF(_F( "%1 at %2:%3.", error, error_line, error_column )));
	}
	// Building a DOM tree <--

	// Parsing XML document -->
	{
		Feed_items_list items;

		QDomElement root = xml.documentElement();

		if(root.tagName() != "feed")
			M_THROW(tr("Invalid XML root element: '%1'."), root.tagName());

		QDomNodeList entries = root.elementsByTagName("entry");

		for(int entry_id = 0; entry_id < entries.size(); entry_id++)
		{
			QDomNode entry = entries.item(entry_id);
			Feed_item item;

			if(entry_id)
				MLIB_DV("");

			// Title and summary -->
				item.title = entry.firstChildElement("title").text();
				MLIB_DV("Title: '%1'.", item.title);

				item.summary = entry.firstChildElement("summary").text();
				MLIB_DV("Summary: '%1'.", item.summary);

				if(item.title.isEmpty() && item.summary.isEmpty())
				{
					// TODO: at less its id
					MLIB_SW(tr("Gotten item with empty title and summary. Skipping it."));
					continue;
				}
			// Title and summary <--

			// Feed info -->
			{
				QDomElement source_dom = entry.firstChildElement("source");

				// Feed URI -->
				{
					QString stream_id = source_dom.attribute("gr:stream-id");
					MLIB_DV("Stream id: %1", stream_id);

					QString stream_id_prefix = "feed/";
					if(
						!stream_id.startsWith(stream_id_prefix) ||
						// http:// or may be https://
						!( item.feed_uri = stream_id.mid(stream_id_prefix.size()) ).startsWith("http")
					)
					{
						// TODO: at less its id
						MLIB_SW(_F( tr("Gotten item with invalid stream id '%1'. Skipping it."), stream_id ));
						continue;
					}
				}
				// Feed URI <--

				// Feed name -->
				{
					QDomNodeList titles = source_dom.elementsByTagName("title");

					MLIB_DV("Titles count: %1.", titles.size());

					if(
						titles.size() < 1 ||
						( item.feed_name = titles.item(0).toElement().text() ).isEmpty()
					)
					{
						// TODO: at less its id
						MLIB_SW(tr("Gotten item with empty subscription name. Skipping it."));
						continue;
					}

					MLIB_DV("Feed name: '%1'.", item.feed_name);
				}
				// Feed name <--
			}
			// Feed info <--

			// Labels -->
			{
				QDomElement entry_dom = entry.toElement();
				QDomNodeList categories = entry_dom.elementsByTagName("category");

				QSet<QString> labels;
				MLIB_DV("Labels:");

				for(int category_id = 0; category_id < categories.size(); category_id++)
				{
					QString label = categories.item(category_id).toElement().attribute("label");

// TODO
					if(!label.isEmpty()/* && label != "reading-list" && label != "fresh"*/)
					{
						MLIB_DV("\t%1", label);
						labels << label;
					}
				}

				Q_FOREACH(const QString& label, labels)
					item.labels << label;
			}
			// Labels <--

			items << item;
		}

		return items;
	}
	// Parsing XML document <--
}


}}}}

