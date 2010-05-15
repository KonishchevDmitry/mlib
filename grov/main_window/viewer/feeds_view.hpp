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


#ifndef GROV_HEADER_VIEWER_FEEDS_VIEW
#define GROV_HEADER_VIEWER_FEEDS_VIEW

class QItemSelection;
class QItemSelectionModel;

#include <QtGui/QTreeView>

#include <grov/common.hpp>

#include <grov/client/storage.hxx>

#include "feeds_model.hxx"


namespace grov { namespace main_window { namespace viewer {


/// Displays a tree of subscriptions and labels.
///
/// This view is not works itself after construction. You must call
/// connect_to_storage() method to start its work.
class Feeds_view: public QTreeView
{
	Q_OBJECT

	public:
		Feeds_view(QWidget *parent = 0);


	private:
		/// All offline data.
		client::Storage*		storage;

		/// Our model that is used to display feed tree.
		Feeds_model*			model;

		/// View's selection model.
		QItemSelectionModel*	selection;


	public:
		/// Connects view to the storage, so it will display its feeds.
		void	connect_to_storage(client::Storage* storage);

		/// Sets selection to none.
		void	select_no_items(void);

	private:
		/// Goes to a next or a previous feed or label with unread items if
		/// such exists.
		void	go_to_unread(bool next);


	signals:
		/// Emits when user selects a feed.
		void	feed_selected(Big_id id);

		/// Emits when user selects a label.
		void	label_selected(Big_id id);

		/// Emits when user selects nothing.
		void	unselected(void);


	public slots:
		/// Goes to a next feed or label with unread items if such exists.
		void	go_to_next_unread(void);

		/// Goes to a previous feed or label with unread items if such exists.
		void	go_to_previous_unread(void);

	private slots:
		/// View's selection changed.
		void	selection_changed(const QItemSelection& selected, const QItemSelection& deselected);
};


}}}

#endif

