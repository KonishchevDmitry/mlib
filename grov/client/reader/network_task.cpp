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


#include <algorithm>

#include <QtCore/QTimer>
#include <QtCore/QUrl>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

#include <grov/common.hpp>
#include <grov/main.hpp>

#include "network_task.hpp"


namespace grov { namespace client { namespace reader {


Network_task::Network_task(QObject* parent)
:
	Task(parent),

	fails_count(0),

	manager(new QNetworkAccessManager(this)),
	current_reply(NULL),
	timeout_timer(new QTimer(this))
{
	this->timeout_timer->setSingleShot(true);
	this->timeout_timer->setInterval(config::network_reply_timeout * 1000);
	connect(this->timeout_timer, SIGNAL(timeout()),
		this, SLOT(on_timeout()) );
}



void Network_task::get(const QString& url)
{
	MLIB_D("Processing a HTTP GET to '%1'...", url);

	QNetworkRequest request = this->prepare_request(url);
	this->process_reply(manager->get(request));
}



void Network_task::on_data_gotten(qint64 size, qint64 total_size)
{
	MLIB_DV("Data gotten: %1 of %2.", size, total_size);

	// If connection is not timed out yet
	if(this->timeout_timer->isActive())
	{
		// Reset the timeout
		this->timeout_timer->stop();
		this->timeout_timer->start();

		// Request's reply size limit -->
		{
			const qint64 max_size = config::max_network_reply_size;
			const qint64 reply_size = std::max(size, total_size);

			if(reply_size > max_size)
			{
				MLIB_D("Request's reply is too big (%1 bytes).", reply_size);

				// TODO: may be format to KB, MB, GB
				this->request_error = _F(tr("Server returned too big amount of data (%1 bytes)."), reply_size);

				// To suppress all queued signals
				this->disconnect(this->current_reply, SIGNAL(downloadProgress(qint64, qint64)), this, NULL);

				this->current_reply->abort();
			}
		}
		// Request's reply size limit <--
	}
	else
	{
		// Connection already timed out, but we had not got a signal yet.
	}
}



void Network_task::on_finished(void)
{
	QNetworkReply* reply = this->current_reply;
	this->current_reply = NULL;

	MLIB_D("Request to '%1' finished.", reply->request().url().toString());
	this->timeout_timer->stop();

	QString reply_error;
	QByteArray reply_data;

	try
	{
		if(!this->request_error.isEmpty())
			M_THROW(this->request_error);

		if(reply->error())
			M_THROW(CSF(reply->errorString()));

		// Checking HTTP status code -->
		{
			int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

			if(code != 200)
				M_THROW("Server returned error: %1 (%2).",
					reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString(), code );
		}
		// Checking HTTP status code <--

		reply_data = reply->readAll();

		if(reply_data.size() < 1000)
			MLIB_DV("Request reply:\n%1", reply_data);
	}
	catch(m::Exception& e)
	{
		this->fails_count++;
		reply_error = EE(e);
	}

	reply->deleteLater();
	this->request_finished(reply_error, reply_data);
}



void Network_task::on_timeout(void)
{
	if(this->current_reply)
	{
		MLIB_D("Request timed out.");
		this->request_error = tr("Connection timed out.");
		this->current_reply->abort();
	}
}



void Network_task::post(const QString& url, const QString& data)
{
	MLIB_D("Processing a HTTP POST to '%1' with data '%2'...", url, data);
	QNetworkRequest request = this->prepare_request(url);
	this->process_reply(this->manager->post(request, data.toAscii()));
}



QNetworkRequest Network_task::prepare_request(const QString& url)
{
	QNetworkRequest request(url);
	request.setRawHeader("User-Agent", get_user_agent().toAscii());
	return request;
}



void Network_task::process_reply(QNetworkReply* reply)
{
	MLIB_A(!this->current_reply);

	this->current_reply = reply;
	this->request_error.clear();
	this->timeout_timer->start();

	connect(reply, SIGNAL(downloadProgress(qint64, qint64)),
		this, SLOT(on_data_gotten(qint64, qint64)) );

	connect(reply, SIGNAL(finished()),
		this, SLOT(on_finished()) );
}



bool Network_task::throw_if_fatal_error(const QString& error)
{
	if(error.isEmpty())
		return false;
	else
	{
		if(this->to_many_tries())
			M_THROW(error);
		else
			return true;
	}
}



bool Network_task::to_many_tries(void)
{
	return this->fails_count >= 3;
}


}}}

