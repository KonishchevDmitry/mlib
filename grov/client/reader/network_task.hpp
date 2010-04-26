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


#ifndef GROV_HEADER_CLIENT_READER_NETWORK_TASK
#define GROV_HEADER_CLIENT_READER_NETWORK_TASK

class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;
class QTimer;

#include <grov/common.hpp>

#include "task.hpp"

#include "network_task.hxx"


namespace grov { namespace client { namespace reader {


/// Base class for all network tasks (which uses QNetworkReply) that we need to
/// process.
class Network_task: public Task
{
	Q_OBJECT

	public:
		Network_task(QObject* parent = NULL);


	private:
		/// Object through which we carry out the networking.
		QNetworkAccessManager*	manager;

		/// QNetworkReply that is processing at this moment.
		QNetworkReply*			current_reply;

		/// Timer to implement timeouts.
		QTimer*					timeout_timer;

		/// Is the timeout for all reply's data or only for each portion of
		/// gotten data.
		bool					overall_timeout;

		/// Error string if request failed.
		QString					request_error;

		/// If true and HTTP status code will be != 200, error will be
		/// returned.
		bool					check_status_code;

		/// Number of failed requests.
		size_t					fails;

		/// Maximum number of allowed fails.
		size_t					max_fails;


	protected:
		/// Sends an HTTP GET request.
		///
		/// @param check_status_code - if true and HTTP status code != 200,
		/// error will be returned.
		void					get(const QString& url, bool check_status_code = true);

		/// Sends an HTTP POST request.
		///
		/// @param check_status_code - if true and HTTP status code != 200,
		/// error will be returned.
		void					post(const QString& url, const QString& data, bool check_status_code = true);

		/// Returns a QNetworkRequest object with the common HTTP headers
		/// and Cookies setted.
		///
		/// We create all requests by this method.
		virtual QNetworkRequest	prepare_request(const QString& url);

		/// get() or post() request finished.
		///
		/// If request failed, \a error will hold error string, otherwise -
		/// error.isEmpty() == true.
		///
		/// When request fails, the fails is incremented.
		virtual void			request_finished(QNetworkReply* reply, const QString& error, const QByteArray& data) = 0;

		/// Resets current fails number.
		void					reset_fails(void);

		/// Sets maximum number of allowed fails.
		void					set_max_fails(size_t fails);

		/// Sets the reply timeout.
		///
		/// @param timeout - timeout in seconds.
		/// @param timeout - is this timeout for all reply's data or only for
		/// each portion of gotten data.
		void					set_timeout(int timeout, bool overall);

		/// This is convenient method for checking the \a error parameter of
		/// request_finished() method.
		///
		/// - If this is error and we already have too many tries, than it
		///   throws m::Exception with this error.
		/// - If this is error and we have free tries, it returns true.
		/// - Otherwise returns false.
		///
		/// @throw m::Exception.
		bool					throw_if_fatal_error(const QString& error);

		/// Returns true if we have too many request's fails and should stop
		/// useless tries.
		bool					to_many_tries(void);

	private:
		/// Starts processing the reply.
		void					process_reply(QNetworkReply* reply, bool check_status_code);


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

