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


#include <QtCore/QtGlobal>

#ifdef Q_OS_UNIX
	#include <fcntl.h>
	#include <signal.h>
	#include <unistd.h>

	#include <boost/foreach.hpp>

	#include <QtCore/QSocketNotifier>
#endif

#include <mlib/core.hpp>

#include "system_signal_notifier.hpp"
#include "system_signal_notifier_private.hpp"


namespace m { namespace sys {


#ifdef Q_OS_UNIX
namespace aux {


	System_signal_notifier* System_signal_notifier::singleton = NULL;



	System_signal_notifier::System_signal_notifier(void)
	{
		MLIB_D("Starting operating system's signals notifier...");

		QString error_title = tr("Operating system signals notifier error");

		try
		{
			if(::pipe(this->pipes))
				M_THROW(PAM( tr("Unable to create a pipe:"), EE() ));

			// Setting non-block mode to prevent application freezing in
			// the case of a huge amount of signals.
			// -->
			{
				long flags = ::fcntl(this->pipes[PIPE_WRITE], F_GETFL, 0);

				if(flags == -1 || ::fcntl(this->pipes[PIPE_WRITE], F_SETFL, flags | O_NONBLOCK) == -1)
					MLIB_SW(error_title,
						PAM( tr("Unable to set non-blocking flag for a pipe:"), EE() ) );
			}
			// <--

			// Setting signal handlers -->
			{
				struct sigaction sig_action;

				::sigemptyset(&sig_action.sa_mask);
				sig_action.sa_flags = SA_RESTART;
				sig_action.sa_handler = &end_work_signal_handler;

				// End an application's work signals -->
				{
					int signal_ids[] = {
						SIGHUP,
						SIGINT,
						SIGQUIT,
						SIGTERM
					};

					BOOST_FOREACH(int signal_id, signal_ids)
						if(::sigaction(signal_id, &sig_action, NULL))
							MLIB_SW(error_title,
								PAM( tr("Unable to set signal [%1] handler:"), EE() ) );
				}
				// End an application's work signals <--
			}
			// Setting signal handlers <--

			// Signal transmitter -->
			{
				QSocketNotifier* notifier = new QSocketNotifier(
					this->pipes[PIPE_READ], QSocketNotifier::Read, this );
				connect(notifier, SIGNAL(activated(int)), this, SIGNAL(end_work()));
			}
			// Signal transmitter <--
		}
		catch(m::Exception& e)
		{
			MLIB_SW(error_title, EE(e));
		}
	}



	void System_signal_notifier::connect_end_work_signal(QObject* object, const char* slot)
	{
		if(!singleton)
			singleton = new System_signal_notifier;

		connect(singleton, SIGNAL(end_work()), object, slot, Qt::QueuedConnection);
	}



	void System_signal_notifier::end_work_signal_handler(int signal_no)
	{
		ssize_t value;
		// To suppress the compiler warning
		value = ::write(singleton->pipes[PIPE_WRITE], &value, 1);
	}


}
#endif



void connect_end_work_system_signal(QObject* object, const char* slot)
{
	MLIB_D("Connecting end application OS signal to the %1::%2...", object, slot);
#ifdef Q_OS_UNIX
	aux::System_signal_notifier::connect_end_work_signal(object, slot);
#else
	#warning TODO: Signal handling for non-UNIX operating systems.
#endif
}


}}

