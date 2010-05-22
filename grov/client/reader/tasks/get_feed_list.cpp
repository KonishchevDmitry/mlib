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


#if GROV_DEVELOP_MODE || GROV_OFFLINE_DEVELOPMENT
	#include <QtCore/QFile>
#endif

#include <QtCore/QUrl>

#include <grov/common.hpp>
#include <grov/main.hpp>

#include <grov/client/storage.hpp>

#include "gr_xml_parser.hpp"

#include "get_feed_list.hpp"


namespace grov { namespace client { namespace reader { namespace tasks {


const char* const Get_feed_list::state_list_names[Get_feed_list::STATE_NUM] = {
	"tag",
	"preference/stream",
	"subscription"
};



Get_feed_list::Get_feed_list(Storage* storage, const QString& auth_id, QObject* parent)
:
	Google_reader_task(auth_id, parent),
	state(STATE_GETTING_TAG_LIST),
	storage(storage)
{
}



void Get_feed_list::authenticated(void)
{
	this->process_current_state();
}



void Get_feed_list::process_current_state(void)
{
	MLIB_D("Getting Google Reader's %1 list...", this->state_list_name());

	#if GROV_OFFLINE_DEVELOPMENT
		QFile list(this->state_list_name() + ".list");

		if(list.open(QIODevice::ReadOnly))
			this->request_finished(NULL, "", list.readAll());
		else
			this->request_finished(NULL, _F("Error while reading '%1'.", list.fileName()), "");
	#else
		QString url;

		switch(this->state)
		{
			case STATE_GETTING_TAG_LIST:
			case STATE_GETTING_STREAM_PREFERENCE_LIST:
			case STATE_GETTING_SUBSCRIPTION_LIST:
				url = "https://www.google.com/reader/api/0/" + this->state_list_name() + "/list";
				break;

			default:
				this->failed(tr("Logical error."));
				return;
				break;
		}

		this->get( url + "?output=xml&client=" + QUrl::toPercentEncoding(get_user_agent()) );
	#endif
}



void Get_feed_list::request_finished(QNetworkReply* reply, const QString& error, const QByteArray& data)
{
	MLIB_D("Google Reader's %1 list request finished.", this->state_list_name());

	try
	{
		try
		{
			// Checking for errors -->
				if(this->throw_if_fatal_error(error))
				{
					MLIB_D("Request failed. Trying again...");
					this->process_current_state();
					return;
				}
			// Checking for errors <--

		#if GROV_DEVELOP_MODE && !GROV_OFFLINE_DEVELOPMENT
			// For offline development -->
			{
				QFile list(this->state_list_name() + ".list");
				list.open(QIODevice::WriteOnly);
				list.write(data);
			}
			// For offline development <--
		#endif

			// Parsing the list -->
				try
				{
					switch(this->state)
					{
						case STATE_GETTING_TAG_LIST:
							this->label_sort_ids = Gr_xml_parser().tag_list(data);
							break;

						case STATE_GETTING_STREAM_PREFERENCE_LIST:
							this->orderings = Gr_xml_parser().stream_preference_list(data);
							break;

						case STATE_GETTING_SUBSCRIPTION_LIST:
							this->feeds = Gr_xml_parser().subscription_list(data);
							break;

						default:
							M_THROW(tr("Logical error."));
							break;
					}
				}
				catch(m::Exception& e)
				{
					M_THROW(PAM( tr("Parsing error."), EE(e) ));
				}
			// Parsing the list <--
		}
		catch(m::Exception& e)
		{
			M_THROW(PAM( _F(tr("Unable to get Google Reader's %1 list."), state_list_name()), EE(e) ));
		}

		// Changing the current state -->
			switch(this->state)
			{
				case STATE_GETTING_TAG_LIST:
				case STATE_GETTING_STREAM_PREFERENCE_LIST:
					this->reset_fails();
					this->state = static_cast<State>(this->state + 1);;
					this->process_current_state();
					break;

				case STATE_GETTING_SUBSCRIPTION_LIST:
				{
					// Throws m::Exception
					this->storage->add_feeds(this->label_sort_ids, this->feeds, this->orderings);

					this->finish();
					emit this->feeds_gotten();
				}
				break;

				default:
					M_THROW(tr("Logical error."));
					break;
			}
		// Changing the current state <--
	}
	catch(m::Exception& e)
	{
		this->failed(EE(e));
	}
}



QString Get_feed_list::state_list_name(void)
{
	return this->state_list_names[this->state];
}


}}}}

