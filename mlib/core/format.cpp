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

#include <mlib/core/types.hpp>

#include "format.hpp"


namespace m {


QString create_sentence_from(QString string)
{
	if(string.isEmpty())
		return string;

	{
		QChar first_letter = string.at(0).toUpper();
		QChar first_upper_letter = first_letter.toUpper();

		if(first_letter != first_upper_letter)
			*string.begin() = first_upper_letter;
	}

	if(string.at(string.size() - 1) != '.')
		string += '.';

	return string;
}



QString pretty_add_message(QString prefix, QString message)
{
	message = message.trimmed();

	// Empty message -->
		if(message.isEmpty())
		{
			if(!prefix.isEmpty())
			{
				if(prefix.endsWith(':'))
					prefix[prefix.size() - 1] = '.';
				else if(!prefix.endsWith('.'))
					prefix += '.';
			}

			return prefix;
		}
	// Empty message <--

	// One line
	if(message.indexOf('\n') < 0)
	{
		if(prefix.endsWith('.'))
		{
			// First letter is upper case
			if(message.begin()->isLower())
				*message.begin() = message.begin()->toUpper();
		}
		else
		{
			// First letter is upper case
			if(message.begin()->isUpper())
				if(message.size() > 1 && message[1].category() == QChar::Letter_Lowercase)
					*message.begin() = message.begin()->toLower();
		}

		prefix += " " + message;

		if(!prefix.endsWith('.'))
			prefix += '.';
	}
	// Many lines
	else
	{
		if(!prefix.endsWith(':') && !prefix.endsWith('.'))
			prefix[prefix.size() - 1] = '.';

		prefix += '\n' + message;
	}

	return prefix;
}


}

