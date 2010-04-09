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


#if MLIB_DEVELOP_MODE
	#include <cstdio>
	#include <cstdlib>
#endif

#include <boost/type_traits/is_arithmetic.hpp>
#include <boost/type_traits/is_enum.hpp>
#include <boost/type_traits/is_pointer.hpp>
#include <boost/type_traits/is_same.hpp>

#include <QtCore/QTextStream>

#include <mlib/core/types.hpp>


namespace m {

namespace format_aux {


	/// Converts value to a string assuming that value is a number;
	template <class Value>
	QString	convert_to_string(const Value value, const boost::true_type&);

	/// Converts value to a string assuming that value is not a number;
	template <class Value>
	QString	convert_to_string(const Value& value, const boost::false_type&);



	template <class Value>
	QString convert_to_string(const Value value, const boost::true_type&)
	{
		return QString::number(value);
	}



	template <class Value>
	QString convert_to_string(const Value& value, const boost::false_type&)
	{
	#if MLIB_DEVELOP_MODE
		// We must try to provide optimal conversion functions for all types we
		// using. So this check allows us to be sure that we not forget about
		// any significant type.
		{
			QTextStream stream(stderr);
			stream
				<< "This code (" << __FILE__ << ':' << __LINE__ << ") "
				<< "must not be reached in the developer mode. So aborting..."
				<< endl;
			abort();
		}
	#endif

		QString string;
		QTextStream stream(&string);
		stream << value;
		return string;
	}


}



QString _F(const QString& format)
{
	return format;
}



template <class A1>
QString _F(const QString& format, const A1& arg_1)
{
	return format.arg(_S(arg_1));
}



template <class A1, class A2>
QString _F(const QString& format, const A1& arg_1, const A2& arg_2)
{
	return format.arg(_S(arg_1), _S(arg_2));
}



template <class A1, class A2, class A3>
QString _F(const QString& format, const A1& arg_1, const A2& arg_2, const A3& arg_3)
{
	return format.arg(_S(arg_1), _S(arg_2), _S(arg_3));
}



template <class A1, class A2, class A3, class A4>
QString _F(const QString& format, const A1& arg_1, const A2& arg_2, const A3& arg_3, const A4& arg_4)
{
	return format.arg(_S(arg_1), _S(arg_2), _S(arg_3), _S(arg_4));
}



template <class Value>
QString _S(const Value& value)
{
	typedef typename boost::is_arithmetic<Value>::type Is_arithmetic;
	typedef typename boost::is_pointer<Value>::type Is_pointer;
	typedef typename boost::is_same<Is_pointer, boost::false_type>::type Not_pointer;
	typedef typename boost::is_same<Is_arithmetic, Not_pointer> Is_number;
	typedef typename boost::is_enum<Value>::type Is_enum;
	typedef typename boost::integral_constant<bool, Is_number::value || Is_enum::value> Is_convertable_to_number;
	return format_aux::convert_to_string(value, Is_convertable_to_number());
}



template <class Value>
QString _S(Value* value)
{
	return "0x" + QString::number(size_t(value), 16).toUpper();
}



QString _S(char symbol)
{
	return QChar(symbol);
}



QString _S(signed char symbol)
{
	return QChar(symbol);
}



QString _S(unsigned char symbol)
{
	return QChar(symbol);
}



QString _S(QChar symbol)
{
	return symbol;
}



QString _S(const char* string)
{
	return string;
}



QString _S(char* string)
{
	return string;
}



QString _S(const QString& string)
{
	return string;
}



QString _S(const QByteArray& string)
{
	return string;
}



QString _S(const std::string& string)
{
	return QString::fromStdString(string);
}



QString CSF(const QString& string)
{
	return m::create_sentence_from(string);
}



QString PAM(const QString& prefix, const QString& message)
{
	return m::pretty_add_message(prefix, message);
}


}

