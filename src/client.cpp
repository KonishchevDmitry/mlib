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
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
**************************************************************************/


#include <src/common.hpp>

#include "client/reader.hpp"
#include "client/storage.hpp"

#include "client.hpp"


namespace grov
{


Client::Client(const QString& user, const QString& password, QObject* parent)
:
	Storage(parent),
	mode(MODE_NONE)
	// TODO: catch Storage exception
{
	// Throws m::Exception
	if(this->has_items())
		this->mode = MODE_OFFLINE;

	this->reader = new client::Reader(this, user, password, this);

	connect(this->reader, SIGNAL(error(const QString&)),
		this, SLOT(on_reader_error(const QString&)) );

	connect(this->reader, SIGNAL(reading_list_gotten()),
		this, SLOT(on_reading_list()) );
}



void Client::change_mode(Mode mode)
{
	this->mode = mode;
	emit this->mode_changed(mode);
}



Client::Mode Client::current_mode(void)
{
	return this->mode;
}



void Client::discard_offline_data(void)
{
	MLIB_A(this->mode == MODE_OFFLINE);

	try
	{
		this->clear();
		this->change_mode(MODE_NONE);
	}
	catch(m::Exception& e)
	{
		MLIB_W(tr("Discarding all offline data failed"), EE(e));
	}
}



void Client::go_offline(void)
{
	MLIB_A(this->mode == MODE_NONE);
	this->change_mode(MODE_GOING_OFFLINE);
	this->reader->get_reading_list();
}



void Client::on_reader_error(const QString& message)
{
	Mode new_mode;
	QString title;

	switch(this->mode)
	{
		case MODE_GOING_OFFLINE:
			// TODO: clear all data
			new_mode = MODE_NONE;
			title = tr("Unable to go offline");
			break;

		default:
			MLIB_LE();
			break;
	}

	MLIB_W(title, message);
	this->change_mode(new_mode);
}



// TODO
void Client::on_reading_list(void)
{
	MLIB_A(this->mode == MODE_GOING_OFFLINE);
	this->change_mode(MODE_OFFLINE);
}


}

