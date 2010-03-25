/**************************************************************************
*                                                                         *
*   grov - Google Reader offline viewer                                   *
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

#include <QtGui/QMainWindow>

#include <src/client.hpp>
#include <src/common.hpp>
#include <src/feeds_model.hxx>


namespace Ui {
    class Main_window;
}

namespace grov {

class Main_window: public QMainWindow
{
	Q_OBJECT

	public:
		Main_window(const QString user, const QString password, QWidget *parent = 0);
		~Main_window(void);


	private:
		Ui::Main_window*	ui;

		/// Represents our Google Reader offline client.
		Client*				client;


		/// Id of item that is being displayed at this time or -1 if no item
		/// is being displayed at this time.
		Big_id				current_item_id;


	protected:
		void	changeEvent(QEvent *e);


	private:
		/// Sets current_item_id to -1.
		void	reset_current_item(void);

		/// Sets current feed item.
		void	set_current_item(const Feed_item& item);

		/// Displays "There is no more unread items" message instead of item.
		void	set_no_more_items(void);


	private slots:
		/// Called when user selects a feed.
		void	feed_selected(Big_id id);

		/// Called when user selects a label.
		void	label_selected(Big_id id);

		/// Called when current mode changed.
		void	mode_changed(Client::Mode mode);

		/// When user clicks "Go offline" button.
		void	on_go_offline_action_activated(void);

		/// When user clicks "Next feed item" button.
		void	on_next_item_action_activated(void);

		/// When user clicks "Previous feed item" button.
		void	on_previous_item_action_activated(void);

		/// Sets items view widget to "Please select a label or a feed" state.
		void	set_no_selected_feed(void);
};

}

#endif

