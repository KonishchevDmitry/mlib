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


#ifndef MLIB_HEADER_CORE_TOOLS
#define MLIB_HEADER_CORE_TOOLS

#include <boost/noncopyable.hpp>
#include <boost/typeof/typeof.hpp>

#if MLIB_DEVELOP_MODE
	#include <QtCore/QDebug>
#endif


/// Returns variable's type.
///
///\code
/// std::vector<int> vec;
/// MLIB_TYPEOF(vec)::iterator vec_iter;
///\endcode
#define MLIB_TYPEOF(variable) m::_tools_aux::Get_type<BOOST_TYPEOF(variable)>::type

/// Returns container's iterator type.
///
/// Example:
/// \code
/// std::vector<int> vec;
/// MLIB_ITER_TYPE(vec) vec_iter;
/// \endcode
#define MLIB_ITER_TYPE(container) MLIB_TYPEOF(container)::iterator

/// Returns container's constant iterator type.
///
/// Example:
/// \code
/// std::vector<int> vec;
/// MLIB_CONST_ITER_TYPE(vec) vec_const_iter;
/// \endcode
#define MLIB_CONST_ITER_TYPE(container) MLIB_TYPEOF(container)::const_iterator


namespace m {


namespace _tools_aux {

	/// For getting variable's type.
	template<class T>
	struct Get_type
	{
		typedef T type;
	};

}


/// Derive your own class from this when you want to create class with virtual
/// destructor.
class Virtual
{
	public:
		virtual ~Virtual(void) {}
};


/// Derive your own class from this when you want to create class with virtual
/// destructor and to prohibit copy construction and copy assignment.
class Virtual_noncopyable: private Virtual, private boost::noncopyable
{
};



/// Calls MLIB_LE() if qobject_cast() fails.
template <class To, class From>
To	checked_qobject_cast(From object);


}


#include "tools.hh"

#endif


