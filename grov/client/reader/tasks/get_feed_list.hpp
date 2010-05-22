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


#ifndef GROV_HEADER_CLIENT_READER_TASKS_GET_FEED_LIST
#define GROV_HEADER_CLIENT_READER_TASKS_GET_FEED_LIST

#include <QtCore/QHash>

#include <grov/common.hpp>
#include <grov/common/feed.hpp>

#include <grov/client/storage.hxx>
#include <grov/client/reader/google_reader_task.hpp>

#include "get_feed_list.hxx"


namespace grov { namespace client { namespace reader { namespace tasks {


/// Gets Google Reader's subscription list.
class Get_feed_list: public Google_reader_task
{
	Q_OBJECT

	private:
		/// Current state.
		enum State {
			/// Now we are getting Google Reader's tag list.
			STATE_GETTING_TAG_LIST,

			/// Now we are getting Google Reader's stream preference list.
			STATE_GETTING_STREAM_PREFERENCE_LIST,

			/// Now we are getting Google Reader's subscription list.
			STATE_GETTING_SUBSCRIPTION_LIST,

			/// Number of states.
			STATE_NUM
		};

		/// State list names.
		static const char* const state_list_names[STATE_NUM];


	public:
		Get_feed_list(Storage* storage, const QString& auth_id, QObject* parent = NULL);


	private:
		/// Current state.
		State					state;

		/// Our offline data storage.
		Storage*				storage;

		/// Label sort ids.
		QHash<QString, QString>	label_sort_ids;

		/// Subscriptions and labels orderings.
		QHash<QString, QString>	orderings;

		/// Feed list.
		Gr_feed_list			feeds;


	public:
		/// See Google_reader_task::authenticated().
		virtual void	authenticated(void);

		/// See Network_task::request_finished().
		virtual void	request_finished(QNetworkReply* reply, const QString& error, const QByteArray& data);

	private:
		/// Processes all actions needed for the current state.
		void			process_current_state(void);

		/// Returns current state list name.
		QString			state_list_name(void);


	signals:
		/// Emitted when we get all feeds.
		void	feeds_gotten(void);
};


}}}}

#endif

