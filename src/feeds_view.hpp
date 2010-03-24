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


// TODO:
#ifndef GROV_HEADER_FEEDS_VIEW
#define GROV_HEADER_FEEDS_VIEW

#include <QtGui/QTreeView>


namespace grov {

class Feeds_view: public QTreeView
{
	Q_OBJECT

	public:
		Feeds_view(QWidget *parent = 0);
#if 0
		~Feeds_view(void);


	private:
		Ui::Feeds_view*	ui;
		Client*				client;


	protected:
		void	changeEvent(QEvent *e);


	private:
		/// Sets current feed item.
		void	set_current_item(const Feed_item& item);

		/// Displays "There is no more unread items" message instead of item.
		void	set_no_more_items(void);


	private slots:
		/// Called when current mode changed.
		void	mode_changed(Client::Mode mode);

		/// When user clicks "Go offline" button.
		void	on_go_offline_action_activated(void);

		/// When user clicks "Next feed item" button.
		void	on_next_item_action_activated(void);

		/// When user clicks "Previous feed item" button.
		void	on_previous_item_action_activated(void);
#endif
};

}

#endif

