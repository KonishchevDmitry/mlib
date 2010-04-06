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


#include <QtNetwork/QNetworkRequest>

#include <src/common.hpp>

#include <src/client/reader.hpp>

#include "tasks/get_gr_token.hpp"

#include "google_reader_task.hpp"


namespace grov { namespace client { namespace reader {


Google_reader_task::Google_reader_task(Reader* reader, QObject* parent)
:
	Network_task(reader, parent)
{
}



void Google_reader_task::get_token(void)
{
	tasks::Get_gr_token* task = new tasks::Get_gr_token(this->reader, this);

	connect(task, SIGNAL(token_gotten(const QString&)),
		this, SLOT(on_token_gotten(const QString&)) );

	task->process();
}



void Google_reader_task::on_token_gotten(const QString& token)
{
	this->token = token;
	this->token_gotten();
}



QNetworkRequest Google_reader_task::prepare_request(const QString& url)
{
	QNetworkRequest request = Network_task::prepare_request(url);
	request.setRawHeader("Authorization", "GoogleLogin auth=" + this->reader->auth_id.toAscii());
	return request;
}



void Google_reader_task::token_gotten(void)
{
}


}}}

