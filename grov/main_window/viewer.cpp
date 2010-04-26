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


#include <QtCore/QUrl>

#include <grov/common.hpp>
#include <grov/common/feed_item.hpp>

#include <grov/client/storage.hpp>
#include <grov/client/web_cache.hpp>
#include <grov/client/web_page.hpp>

#include "viewer.hpp"
#include "ui_viewer.h"


namespace grov { namespace main_window {


Viewer::Viewer(QWidget *parent)
:
	QWidget(parent),
    ui(new Ui::Viewer),
	storage(NULL)
{
	ui->setupUi(this);

	ui->item_view->setPage(new client::Web_page(ui->item_view));
	ui->item_view->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
	connect(ui->item_view, SIGNAL(linkClicked(const QUrl&)),
		this, SLOT(link_clicked(const QUrl&)) );

	ui->star_check_box->addAction(ui->star_action);
	this->set_no_selected_feed();
}



Viewer::~Viewer(void)
{
	delete ui;
}



void Viewer::connect_to_storage(client::Storage* storage)
{
	MLIB_A(!this->storage);
	this->storage = storage;

	ui->item_view->page()->setNetworkAccessManager(
		new client::Web_cached_manager(this->storage, this) );

	// Feeds_view -->
		ui->feeds_view->connect_to_storage(this->storage);

		connect(ui->feeds_view, SIGNAL(unselected()),
			this, SLOT(set_no_selected_feed()) );

		connect(ui->feeds_view, SIGNAL(feed_selected(Big_id)),
			this, SLOT(feed_selected(Big_id)) );

		connect(ui->feeds_view, SIGNAL(label_selected(Big_id)),
			this, SLOT(label_selected(Big_id)) );
	// Feeds_view <--
}



void Viewer::feed_selected(Big_id id)
{
	this->storage->set_current_source_to_feed(id);
	this->reset_current_item();
	this->go_to_next_item(true);
}



void Viewer::go_to_next_item(bool source_changed)
{
	try
	{
		if(this->current_item.valid())
			this->storage->mark_as_read(this->current_item);
	}
	catch(m::Exception& e)
	{
		if(source_changed)
			this->set_no_selected_feed();

		MLIB_W(tr("Unable to mark feed's item as read"), EE(e));
		return;
	}

	try
	{
		Db_feed_item item = this->storage->get_next_item();
		this->set_current_item(item);
	}
	catch(client::Storage::No_more_items&)
	{
		this->set_no_more_items();
	}
	catch(client::Storage::No_selected_items&)
	{
		this->set_no_selected_feed();
	}
	catch(m::Exception& e)
	{
		if(source_changed)
			this->set_no_selected_feed();

		MLIB_W(tr("Unable to fetch feed's item"), EE(e));
	}
}



void Viewer::go_to_previous_item(void)
{
	try
	{
		Db_feed_item item = this->storage->get_previous_item();
		this->set_current_item(item);
	}
	catch(client::Storage::No_more_items&) { }
	catch(client::Storage::No_selected_items&) { }
	catch(m::Exception& e)
	{
		MLIB_W(tr("Unable to fetch feed's item"), EE(e));
	}
}



void Viewer::label_selected(Big_id id)
{
	this->storage->set_current_source_to_label(id);
	this->reset_current_item();
	this->go_to_next_item(true);
}



void Viewer::link_clicked(const QUrl& qurl)
{
	QString url = qurl.toString();
	MLIB_D("User clicked link '%1'.", url);

	if(!url.isEmpty() && url == this->current_item.url)
	{
		try
		{
			if(this->storage->is_in_web_cache(url))
			{
				MLIB_D("This is item's cached page. Loading it...");
				ui->item_view->load(url);
			}
		}
		catch(m::Exception& e)
		{
			MLIB_W(tr("Error while loading clicked link's page"), EE(e));
		}
	}
}



void Viewer::on_star_check_box_stateChanged(int state)
{
	try
	{
		this->storage->star(this->current_item.id, state);
	}
	catch(m::Exception& e)
	{
		// Giving back old value
		ui->star_check_box->blockSignals(true);
		ui->star_check_box->setChecked(!state);
		ui->star_check_box->blockSignals(false);

		MLIB_W(tr("Unable to star feed's item"), EE(e));
	}
}



void Viewer::reset_current_item(void)
{
	this->current_item.set_invalid();
	ui->controls_box->setVisible(false);
}



void Viewer::select_no_feed(void)
{
	ui->feeds_view->select_no_items();
}



// TODO
//void Viewer::set_current_item(const Db_feed_item& item)
void Viewer::set_current_item(Db_feed_item item)
{
	// TODO
//	item.url = "http://server.lab83/papercraft/test/" + QString::number(item.id) + "/";
	ui->item_view->setHtml(_F(
		"<html><body>"
			"<a href='%1'><h1 style='font-size: 14pt'>%2</h1></a>"
			"%3"
		"</body></html>",
		QUrl::toPercentEncoding(item.url), item.title, item.summary
	));

	this->current_item = item;

	ui->star_check_box->blockSignals(true);
	ui->star_check_box->setChecked(item.starred);
	ui->star_check_box->blockSignals(false);
	ui->controls_box->setVisible(true);
}



void Viewer::set_no_more_items(void)
{
	QString html;

	html += "<html><body><center>";
	html += tr("You have no unread items.");
	html += "</center></body></html>";

	ui->item_view->setHtml(html);

	this->reset_current_item();
}



void Viewer::set_no_selected_feed(void)
{
	QString html;

	html += "<html><body><center>";
	html += tr("Please select a label or a feed to view its items.");
	html += "</center></body></html>";

	ui->item_view->setHtml(html);

	this->reset_current_item();
}


}}

