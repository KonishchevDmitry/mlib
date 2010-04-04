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


#ifndef GROV_HEADER_MAIN_WINDOW_VIEWER
#define GROV_HEADER_MAIN_WINDOW_VIEWER

#include <QtGui/QWidget>

#include <src/common.hpp>
#include <src/common/feed_item.hpp>

#include <src/client/storage.hxx>


namespace grov { namespace main_window {


namespace Ui {
	class Viewer;
}


/// A widget that displays all ours offline data.
///
/// This viewer is not works itself after construction. You must call
/// connect_to_storage() method to start its work.
class Viewer: public QWidget
{
	Q_OBJECT

	public:
		Viewer(QWidget *parent = 0);
		~Viewer(void);


	private:
		// Qt Designer-generated widgets.
		Ui::Viewer*			ui;

		/// All offline data.
		client::Storage*	storage;


		/// Item that is being displayed at this time or invalid item if not
		/// item is being displayed at this time.
		Feed_item			current_item;


	public:
		/// Connects viewer to the storage, so it will display its data.
		void	connect_to_storage(client::Storage* storage);

	private:
		/// Sets current_item.id to -1.
		void	reset_current_item(void);

		/// Sets current feed item.
		void	set_current_item(const Feed_item& item);

		/// Displays "There is no more unread items" message instead of item.
		void	set_no_more_items(void);


	public slots:
		/// When user clicks "Next feed item" button.
		void	go_to_next_item(void);

		/// When user clicks "Previous feed item" button.
		void	go_to_previous_item(void);

	private slots:
		/// Called when user selects a feed.
		void	feed_selected(Big_id id);

		/// Called when user selects a label.
		void	label_selected(Big_id id);

		/// Called when user stars/unstars an item.
		void	on_star_check_box_stateChanged(int state);

		/// Sets items view widget to "Please select a label or a feed" state.
		void	set_no_selected_feed(void);
};


}}

#endif

