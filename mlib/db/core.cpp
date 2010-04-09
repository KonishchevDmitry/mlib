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
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
**************************************************************************/


#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

#include <mlib/core.hpp>

#include "core.hpp"


namespace m {


QString EE(const QSqlDatabase& db)
{
	return EE(db.lastError());
}



// TODO: test
QString EE(const QSqlError& error)
{
	QString database_text = error.databaseText();
	QString driver_text = error.driverText();

	if(database_text == driver_text)
		return database_text;
	else
		return driver_text + " (" + database_text + ')';
}



QString EE(const QSqlQuery& query)
{
	return EE(query.lastError());
}


}

