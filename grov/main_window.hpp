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


#ifndef GROV_HEADER_MAIN_WINDOW
#define GROV_HEADER_MAIN_WINDOW

class QMessageBox;

#include <boost/scoped_ptr.hpp>

#include <QtGui/QMainWindow>

#include <mlib/gui/message_box.hxx>

#include <grov/client.hpp>
#include <grov/common.hpp>


namespace grov {


namespace Ui {
	class Main_window;
}


/// Application's main window.
class Main_window: public QMainWindow
{
	Q_OBJECT

	public:
		/// @throw m::Exception
		Main_window(QWidget *parent = 0);
		~Main_window(void);


	private:
		// Qt Designer-generated widgets.
		boost::scoped_ptr<Ui::Main_window>	ui;

		/// Represents our Google Reader offline client.
		Client*								client;

		/// Dialog that shows progress of current operation.
		m::gui::Message_box*				progress_dialog;


	private slots:
		/// Called when current mode changed.
		void	mode_changed(Client::Mode mode);

		/// When user clicks "About" button.
		void	on_about_activated(void);

		/// When user clicks "Discard all offline data" button.
		void	on_discard_all_offline_data_activated(void);

		/// When user clicks "Flush offline data" button.
		void	on_flush_offline_data_activated(void);

		/// When user clicks "Go offline" button.
		void	on_go_offline_activated(void);
};


}

#endif

