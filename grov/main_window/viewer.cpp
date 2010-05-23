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

#include <QtGui/QMenu>

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
	go_to_page_action(NULL),
	star_action(NULL),
	storage(NULL)
{
	ui->setupUi(this);

	ui->item_view->setPage(new client::Web_page(ui->item_view));
	ui->item_view->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
	#warning
//	ui->item_view->settings()->setFontSize(QWebSettings::MinimumFontSize, config::min_font_size);
	ui->item_view->settings()->setFontSize(QWebSettings::MinimumFontSize, 14);
	ui->item_view->settings()->setFontSize(QWebSettings::DefaultFontSize, 14);
	ui->item_view->settings()->setFontSize(QWebSettings::DefaultFixedFontSize, 14);
	connect(ui->item_view, SIGNAL(linkClicked(const QUrl&)),
		SLOT(link_clicked(const QUrl&)) );

	connect(this, SIGNAL(item_selected(bool)),
		SLOT(item_selected_cb(bool)) );

	this->set_no_selected_feed();
}



Viewer::~Viewer(void)
{
	delete ui;
}



void Viewer::connect_to_parent(client::Storage* storage, QAction* go_to_page_action, QAction* star_action)
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

	// Item's actions -->
		// Go to item's page -->
			this->go_to_page_action = go_to_page_action;

			connect(this->go_to_page_action, SIGNAL(activated()),
				SLOT(go_to_item_page()) );
		// Go to item's page <--

		// Star item -->
			this->star_action = star_action;
			this->star_action->setChecked(ui->star_check_box->isChecked());

			connect(this->star_action, SIGNAL(activated()),
				ui->star_check_box, SLOT(toggle()) );
		// Star item <--
	// Item's actions <--

	// To generate all signals that we need to fully synchronize with parent
	// widget.
	this->set_no_selected_feed();
}



void Viewer::feed_selected(Big_id id)
{
	this->storage->set_current_source_to_feed(id);
	this->reset_current_item();
	this->go_to_next_item(true);
}



void Viewer::go_to_item_page(void)
{
	if(this->current_item.valid() && !this->current_item.url.isEmpty())
		this->link_clicked(this->current_item.url);
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



void Viewer::go_to_next_unread_feed_or_label(void)
{
	ui->feeds_view->go_to_next_unread();
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



void Viewer::go_to_previous_unread_feed_or_label(void)
{
	ui->feeds_view->go_to_previous_unread();
}



void Viewer::item_selected_cb(bool valid)
{
	if(this->go_to_page_action)
		this->go_to_page_action->setEnabled(valid && !this->current_item.url.isEmpty());

	if(this->star_action)
		this->star_action->setEnabled(valid);
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

				// Ignoring all following requests.
				if(this->go_to_page_action)
					this->go_to_page_action->setEnabled(false);

				ui->item_view->load(url);
			}
		}
		catch(m::Exception& e)
		{
			MLIB_W(tr("Error while loading clicked link's page"), EE(e));
		}
	}
}



void Viewer::on_star_check_box_toggled(bool state)
{
	if(this->star_action)
		this->star_action->setChecked(state);

	try
	{
		this->storage->star(this->current_item.id, state);
	}
	catch(m::Exception& e)
	{
		// Giving back the old value
		this->set_star_flag_to(!state);

		MLIB_W(tr("Unable to star feed's item"), EE(e));
	}
}



void Viewer::reset_current_item(void)
{
	this->set_current_item(Db_feed_item());
}



void Viewer::select_no_feed(void)
{
	ui->feeds_view->select_no_items();
}



void Viewer::set_current_item(const Db_feed_item& item)
{
	bool valid = item.valid();

	this->current_item = item;
	ui->controls_box->setVisible(valid);

	if(valid)
	{
		ui->item_view->setHtml(_F(
			"<html><body>"
				"<a href='%1'><h1 style='font-size: 14pt'>%2</h1></a>"
				"%3"
			"</body></html>",
			item.url, item.title, item.summary
		));

		this->set_star_flag_to(item.starred);
	}

	emit this->item_selected(valid);
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



void Viewer::set_star_flag_to(bool starred)
{
	ui->star_check_box->blockSignals(true);
	ui->star_check_box->setChecked(starred);
	ui->star_check_box->blockSignals(false);

	if(this->star_action)
		this->star_action->setChecked(starred);
}


}}

