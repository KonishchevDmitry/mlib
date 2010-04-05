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
#include <src/common/feed.hpp>
#include <src/common/feed_item.hpp>

#include "gr_xml_parser.hpp"


namespace grov { namespace client { namespace reader { namespace tasks {


QDomDocument Gr_xml_parser::get_dom(const QByteArray& data)
{
	QDomDocument xml;
	QString error;
	int error_line;
	int error_column;

	if(!xml.setContent(data, false, &error, &error_line, &error_column))
		M_THROW(CSF(_F( "%1 at %2:%3.", error, error_line, error_column )));

	return xml;
}



// Check and clean
Gr_feed_item_list Gr_xml_parser::reading_list(const QByteArray& data, QString* continuation_code)
{
	MLIB_DV("Parsing feeds' items list:");

	// Throws m::Exception
	QDomDocument xml = this->get_dom(data);

	// Parsing XML document -->
	{
		Gr_feed_item_list items;

		QDomElement root = xml.documentElement();

		if(root.tagName() != "feed")
			M_THROW(tr("Invalid XML root element: '%1'."), root.tagName());

		// Continuation code -->
			if(continuation_code)
			{
				QDomNodeList codes = root.elementsByTagName("gr:continuation");

				MLIB_DV("Number of continuation codes: %1.", codes.size());

				if(codes.isEmpty())
					continuation_code->clear();
				else
				{
					*continuation_code = codes.item(0).toElement().text();
					MLIB_DV("Continuation code: '%1'.", *continuation_code);
				}
			}
		// Continuation code <--

		QDomNodeList entries = root.elementsByTagName("entry");

		for(int entry_id = 0; entry_id < entries.size(); entry_id++)
		{
			QDomNode entry = entries.item(entry_id);
			Gr_feed_item item;

			if(entry_id)
				MLIB_DV("");

			// Google Reader's id -->
				item.gr_id = entry.firstChildElement("id").text();
				MLIB_DV("Google Reader's id: '%1'.", item.gr_id);

				if(item.gr_id.isEmpty())
					M_THROW(tr("Gotten item with empty id."));
			// Google Reader's id <--

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

				// Feed's id -->
				{
					QString stream_id = source_dom.attribute("gr:stream-id");
					MLIB_DV("Stream id: %1", stream_id);

					// TODO: check
					item.feed_gr_id = stream_id;
					#if 0
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
					#endif
				}
				// Feed's id <--

				// Feed name -->
				{
					QDomNodeList titles = source_dom.elementsByTagName("title");

					MLIB_DV("Titles count: %1.", titles.size());

					if(
						titles.isEmpty() ||
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
					if(!label.isEmpty() /*&& label != "reading-list" && label != "fresh"*/)
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



Gr_feed_list Gr_xml_parser::subscription_list(const QByteArray& data)
{
	MLIB_DV("Parsing subscriptions list:");

	// Throws m::Exception
	QDomDocument xml = this->get_dom(data);

	Gr_feed_list feeds;
	QDomNode feed_node;

	// Getting feed nodes -->
	{
		QDomElement root = xml.documentElement();
		if(root.tagName() != "object")
			M_THROW(tr("Invalid XML root element: '%1'."), root.tagName());

		QDomNodeList lists = root.elementsByTagName("list");
		for(int id = 0; id < lists.size(); id++)
		{
			QDomNode list = lists.item(id);

			if(list.toElement().attribute("name") == "subscriptions")
			{
				feed_node = list.firstChild();
				break;
			}
		}
	}
	// Getting feed nodes <--

	// Parsing feed nodes -->
		while(!feed_node.isNull())
		{
			Gr_feed feed;
			QDomNode prop_node = feed_node.firstChild();

			while(!prop_node.isNull())
			{
				QDomElement prop = prop_node.toElement();

				if(prop.tagName() == "string")
				{
					if(prop.attribute("name") == "id")
					{
						feed.gr_id = prop.text();
						MLIB_DV("Id: '%1'.", feed.gr_id);
					}
					else if(prop.attribute("name") == "title")
					{
						feed.name = prop.text();
						MLIB_DV("Title: '%1'.", feed.name);
					}
				}
				else if(prop.tagName() == "list" && prop.attribute("name") == "categories")
				{
					QDomNodeList categories = prop.elementsByTagName("string");

					for(int id = 0; id < categories.size(); id++)
					{
						QDomElement category = categories.item(id).toElement();

						if(category.tagName() == "string" && category.attribute("name") == "label")
						{
							QString label = category.text();
							MLIB_DV("Label: '%1'.", label);
							feed.labels << label;
						}
					}
				}

				prop_node = prop_node.nextSibling();
			}

			if(feed.gr_id.isEmpty())
				M_THROW(tr("Given subscription has no id."));

			if(feed.name.isEmpty())
				feed.name = tr("(title unknown)");

			feeds << feed;

			feed_node = feed_node.nextSibling();

			MLIB_DV("");
		}
	// Parsing feed nodes <--

	MLIB_DV("Subscriptions list parsed.");

	return feeds;
}


}}}}

