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


#include <algorithm>

#include <QtCore/QTimer>
#include <QtCore/QUrl>

#include <QtNetwork/QNetworkAccessManager>
// TODO
#include <QtNetwork/QNetworkCookie>
// TODO
#include <QtCore/QVariant>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

#include <src/common.hpp>

#include "network_task.hpp"


namespace grov { namespace client { namespace reader {


Network_task::Network_task(const QNetworkRequest& request_template, QObject* parent)
:
	Task(parent),

	request_template(request_template),
	current_reply(NULL),
	fails_count(0),

	manager(new QNetworkAccessManager(this)),
	timeout_timer(new QTimer(this)),
	downloaded_bytes(0)
{
	this->timeout_timer->setSingleShot(true);
	// TODO
	this->timeout_timer->setInterval(10 * 1000);
	connect(this->timeout_timer, SIGNAL(timeout()),
		this, SLOT(on_timeout()) );
}



void Network_task::get(const QString& url)
{
	MLIB_D("Processing a HTTP GET to '%1'...", url);

	QNetworkRequest request = this->request_template;
	request.setUrl(url);

	this->process_reply(manager->get(request));
}



void Network_task::on_data_gotten(qint64 size, qint64 total_size)
{
	MLIB_DV("Data gotten: %1 of %2.", size, total_size);
	this->downloaded_bytes += size;

	// If connection is not timed out yet
	if(this->timeout_timer->isActive())
	{
		// Reset the timeout
		this->timeout_timer->stop();
		this->timeout_timer->start();

		// Request's reply size limit -->
		{
			// TODO
			const qint64 max_size = 10 * 1024 * 1024;
			const qint64 reply_size = std::max(this->downloaded_bytes, total_size);

			if(reply_size > max_size)
			{
				MLIB_D("Request's reply is too big (%1 bytes).", reply_size);
				// TODO: may be format to KB, MB, GB
				this->reply_error = _F(tr("Server returned too big amount of data (%1 bytes)."), reply_size);
				this->current_reply->abort();
			}
		}
		// Request's reply size limit <--
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
		if(!this->reply_error.isEmpty())
			M_THROW(this->reply_error);

		if(reply->error())
			M_THROW(CSF(reply->errorString()));

		// Checking HTTP status code -->
		{
			int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

			if(code != 200)
				M_THROW(PAM("Server returned error:", _F("%1 (%2).",
					reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString(), code) ));
		}
		// Checking HTTP status code <--

		reply_data = reply->readAll();
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
	MLIB_D("Request timed out.");
	this->reply_error = tr("Connection timed out.");
	this->current_reply->abort();
}



void Network_task::post(const QNetworkRequest& request, const QByteArray& data)
{
	MLIB_D("Processing a HTTP POST to '%1'...", request.url().toString());
	this->process_reply(this->manager->post(request, data));
}



void Network_task::process_reply(QNetworkReply* reply)
{
	MLIB_A(!this->current_reply);

	this->current_reply = reply;
	this->downloaded_bytes = 0;
	this->reply_error.clear();
	this->timeout_timer->start();

	connect(reply, SIGNAL(downloadProgress(qint64, qint64)),
		this, SLOT(on_data_gotten(qint64, qint64)) );

	connect(reply, SIGNAL(finished()),
		this, SLOT(on_finished()) );
}



bool Network_task::to_many_tries(void)
{
	return this->fails_count >= 3;
}


}}}

