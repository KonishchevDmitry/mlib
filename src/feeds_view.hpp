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


#ifndef GROV_HEADER_FEEDS_VIEW
#define GROV_HEADER_FEEDS_VIEW

class QItemSelection;
class QItemSelectionModel;

#include <QtGui/QTreeView>

#include <src/feeds_model.hxx>
#include <src/storage.hxx>


namespace grov {

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
		Storage*				storage;

		/// Our model that is used to display feed tree.
		Feeds_model*			model;

		/// View's selection model.
		QItemSelectionModel*	selection;


	public:
		/// Connects view to the storage, so it will display its feeds.
		void	connect_to_storage(Storage* storage);


	signals:
		/// Emits when user selects a feed.
		void	feed_selected(Big_id id);

		/// Emits when user selects a label.
		void	label_selected(Big_id id);

		/// Emits when user selects nothing.
		void	unselected(void);


	private slots:
		/// View's selection changed.
		void	selection_changed(const QItemSelection& selected, const QItemSelection& deselected);



};

}

#endif

