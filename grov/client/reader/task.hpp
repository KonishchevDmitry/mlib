/**************************************************************************
*                                                                         *
*   Grov - Google Reader offline viewer                                   *
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


#ifndef GROV_HEADER_CLIENT_READER_TASK
#define GROV_HEADER_CLIENT_READER_TASK

#include <grov/common.hpp>

#include "task.hxx"


namespace grov { namespace client { namespace reader {


/// Base class for all tasks that we need to process.
class Task: public QObject
{
	Q_OBJECT

	public:
		Task(QObject* parent = NULL);
		~Task(void);


	private:
		/// Is task finished.
		bool	finished;


	public:
		/// Processes the task.
		virtual void	process(void) = 0;

	protected:
		/// Returns true if task cancelled.
		///
		/// @ATTENTION
		/// Must be used only in destructors - in other methods the return
		/// value is undefined.
		bool			is_cancelled(void);

		/// Emits error() signal end finish the task.
		void			failed(const QString& message);

		/// Asynchronously deletes the task.
		void			finish(void);

		/// Processes a child task.
		void			process_task(Task* task);


	signals:
		/// Emitted when task cancelled.
		void	cancelled(void);

		/// This signal task emits when processing fails.
		void	error(const QString& message);


	public slots:
		/// Cancels the task.
		virtual void	cancel(void);

	private slots:
		/// Called when child task emits error() signal.
		void			child_task_error(const QString& message);

};


}}}

#endif

