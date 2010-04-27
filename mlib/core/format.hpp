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


#ifndef MLIB_HEADER_CORE_FORMAT
#define MLIB_HEADER_CORE_FORMAT

#include <mlib/core/types.hpp>


namespace m {


/// Formats the string.
inline QString	_F(const QString& format);

/// Formats the string.
template <class A1>
QString			_F(const QString& format, const A1& arg_1);

/// Formats the string.
template <class A1, class A2>
QString			_F(const QString& format, const A1& arg_1, const A2& arg_2);

/// Formats the string.
template <class A1, class A2, class A3>
QString			_F(const QString& format, const A1& arg_1, const A2& arg_2, const A3& arg_3);

/// Formats the string.
template <class A1, class A2, class A3, class A4>
QString			_F(const QString& format, const A1& arg_1, const A2& arg_2, const A3& arg_3, const A4& arg_4);

/// Formats the string.
template <class A1, class A2, class A3, class A4, class A5>
QString			_F(const QString& format, const A1& arg_1, const A2& arg_2, const A3& arg_3, const A4& arg_4, const A5& arg_5);


/// Converts value to a string.
template <class Value>
QString			_S(const Value& value);

/// Converts pointer to a string.
template <class Value>
QString			_S(Value* value);

/// Specialization of _S().
inline QString	_S(char symbol);

/// Specialization of _S().
inline QString	_S(signed char symbol);

/// Specialization of _S().
inline QString	_S(unsigned char symbol);

/// Specialization of _S().
inline QString	_S(QChar symbol);

/// Specialization of _S().
inline QString	_S(const char* string);

/// Specialization of _S().
inline QString	_S(char* string);

/// Specialization of _S().
inline QString	_S(const QString& string);

/// Specialization of _S().
inline QString	_S(const QByteArray& string);

/// Specialization of _S().
inline QString	_S(const std::string& string);


/// Creates a sentence from a given string.
///
/// create_sentence_from("some error message") returns "Some error message.".
QString			create_sentence_from(QString string);

/// Alias to create_sentence_from().
inline QString	CSF(const QString& string);

/// Formats message so it can be placed as: "$prefix:$message".
///
/// The results are:
/// <pre>
/// prefix == "Unable to complete task:"
///
/// message == "Connection timed out":
/// Unable to complete task: connection timed out.
///
/// message == "Connection timed out. Please check your network settings.":
/// Unable to complete task: connection timed out. Please check your network settings.
///
/// message == "I/O error":
/// Unable to complete task: I/O error.
///
/// message == "Can't open files for reading:\nUnable to open file foo.png: I/O error.\nUnable to open file bar.png: I/O error.":
/// Unable to complete task. Can't open files for reading:
/// Unable to open file foo.png: I/O error.
/// Unable to open file bar.png: I/O error.
///
///
/// prefix == "Unable to complete task."
///
/// message == "Connection timed out":
/// Unable to complete task. Connection timed out.
/// </pre>
QString			pretty_add_message(QString prefix, QString message);

/// Alias to pretty_add_message().
inline QString	PAM(const QString& prefix, const QString& message);


}

#if MLIB_ENABLE_ALIASES
	using m::_F;
	using m::_S;
	using m::CSF;
	using m::PAM;
#endif

#include "format.hh"

#endif

