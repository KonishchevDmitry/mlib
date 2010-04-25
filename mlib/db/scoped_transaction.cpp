/**************************************************************************
*                                                                         *
*   MLib - library of some useful things for internal usage               *
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


#include <mlib/core.hpp>

#include "core.hpp"

#include "scoped_transaction.hpp"


namespace m { namespace db {


Scoped_transaction::Scoped_transaction(const QSqlDatabase& db)
:
	db(db),
	closed(false)
{
	MLIB_D("Starting a transaction...");

	if(!this->db.transaction())
		M_THROW(EE(this->db));
}



Scoped_transaction::~Scoped_transaction(void)
{
	if(!this->closed)
	{
		MLIB_D("Rolling back the transaction...");

		if(!this->db.rollback())
			MLIB_SW(PAM( tr("Unable to rollback a transaction:"), EE(this->db) ));
	}
}



void Scoped_transaction::commit(void)
{
	MLIB_D("Committing the transaction...");

	if(!this->db.commit())
		M_THROW(EE(this->db));

	this->closed = true;
}


}}

