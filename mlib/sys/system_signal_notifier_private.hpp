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


#ifndef MLIB_HEADER_SYS_SYSTEM_SIGNAL_NOTIFIER_PRIVATE
#define MLIB_HEADER_SYS_SYSTEM_SIGNAL_NOTIFIER_PRIVATE

#include <mlib/core.hpp>


namespace m { namespace sys { namespace aux {


#ifdef Q_OS_UNIX
	/// "Translates" the operating system's signals to Qt's signals.
	class System_signal_notifier: public QObject
	{
		Q_OBJECT

		private:
			/// Pipes' ids.
			enum {
				/// Read pipe.
				PIPE_READ,

				/// Write pipe.
				PIPE_WRITE,

				/// Pipes number.
				PIPE_NUM
			};


		public:
			System_signal_notifier(void);


		private:
			/// Notifier's singleton.
			static System_signal_notifier*	singleton;

			/// Pipes through which we transmit the operating system's signals
			/// to Qt signals.
			int								pipes[PIPE_NUM];


		public:
			/// Connects a slot to the operating system's signals that
			/// terminates an application.
			static void	connect_end_work_signal(QObject* object, const char* slot);

		private:
			/// Called by the operating system when we get a signal that
			/// terminates an application.
			static void	end_work_signal_handler(int signal_no);


		signals:
			/// Emitted when we get a signal from the operating system with
			/// terminates an application.
			void	end_work(void);
	};
#endif


}}}

#endif

