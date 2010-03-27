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


#ifndef GROV_HEADER_CLIENT_READER_NETWORK_TASK
#define GROV_HEADER_CLIENT_READER_NETWORK_TASK

class QNetworkAccessManager;
class QNetworkReply;
class QTimer;

#include <QtNetwork/QNetworkRequest>

#include <src/common.hpp>

#include "task.hpp"

#include "network_task.hxx"


namespace grov { namespace client { namespace reader {


/// Base class for all network tasks (which uses QNetworkReply) that we need to
/// process.
class Network_task: public Task
{
	Q_OBJECT

	public:
		Network_task(const QNetworkRequest& request_template, QObject* parent = NULL);


	protected:
		/// QNetworkRequest template.
		///
		/// We create all requests based on this template.
		QNetworkRequest	request_template;

		/// QNetworkReply that is processing at this moment.
		QNetworkReply*	current_reply;

		/// Number of failed requests.
		size_t			fails_count;

	private:
		/// Object through which we carry out the networking.
		QNetworkAccessManager*	manager;

		/// Timer to implement timeouts.
		QTimer*					timeout_timer;

		/// Data size which we already downloaded.
		qint64					downloaded_bytes;

		/// Error string if request failed.
		QString					reply_error;


	protected:
		/// Sends an HTTP GET request.
		void			get(const QString& url);

		/// Sends an HTTP POST request.
		void			post(const QNetworkRequest& request, const QByteArray& data);

		/// get() or post() request finished.
		///
		/// If request failed, \a error will hold error string, otherwise -
		/// error.isEmpty() == true.
		///
		/// When request fails, the fails_count is incremented.
		virtual void	request_finished(const QString& error, const QByteArray& reply) = 0;

		/// Returns true if we have to many request's fails and should stop
		/// useless tries.
		bool			to_many_tries(void);

	private:
		/// Starts processing the reply.
		void			process_reply(QNetworkReply* reply);


	private slots:
		/// Emits when we get a data.
		void	on_data_gotten(qint64 size, qint64 total_size);

		/// Emits when QNetworkReply finishes.
		void	on_finished(void);

		/// When timeout for task expires.
		void	on_timeout(void);

};


}}}

#endif

