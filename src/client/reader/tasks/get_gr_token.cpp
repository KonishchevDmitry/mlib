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


#include <src/common.hpp>

#include "get_gr_token.hpp"


namespace grov { namespace client { namespace reader { namespace tasks {


Get_gr_token::Get_gr_token(Reader* reader, QObject* parent)
:
	Google_reader_task(reader, parent)
{
}



void Get_gr_token::request_finished(const QString& error, const QByteArray& reply)
{
	MLIB_D("Google Reader's API token request finished.");

	try
	{
		try
		{
			// Checking for errors -->
				if(this->throw_if_fatal_error(error))
				{
					MLIB_D("Request failed. Trying again...");
					this->process();
					return;
				}
			// Checking for errors <--

			if(reply.isEmpty())
				M_THROW(tr("Gotten empty token."));
		}
		catch(m::Exception& e)
		{
			M_THROW(PAM( tr("Unable to get Google Reader's API token."), EE(e) ));
		}

		MLIB_D("Gotten Google Reader's API token: '%1'.", reply);
		emit this->token_gotten(reply);
	}
	catch(m::Exception& e)
	{
		emit this->error(EE(e));
	}
}



void Get_gr_token::process(void)
{
	MLIB_D("Getting Google Reader's API token...");

#if OFFLINE_DEVELOPMENT
	emit this->token_gotten("fake offline token");
#else
	// TODO: more params
	QString query = "https://www.google.com/reader/api/0/token";
	this->get(query);
#endif
}


}}}}

