/*
 * Copyright © 2004-2010 Jens Oknelid, paskharen@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * In addition, as a special exception, compiling, linking, and/or
 * using OpenSSL with this program is allowed.
 */

#include "bookentry.hh"
#include "wulformanager.hh"
#include "settingsmanager.hh"

using namespace std;

GSList* BookEntry::group = NULL;

BookEntry::BookEntry(const EntryType type, const string &text, const string &ui, const string &id):
    Entry(type, ui, id),
    bold(FALSE),
    urgent(FALSE)
{
#if GTK_CHECK_VERSION(3, 2, 0)
    labelBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,5);
#else
    labelBox = gtk_hbox_new(FALSE, 5);
#endif


    eventBox = gtk_event_box_new();
    gtk_event_box_set_above_child(GTK_EVENT_BOX(eventBox), TRUE);
    gtk_event_box_set_visible_window(GTK_EVENT_BOX(eventBox), FALSE);

        // icon
        icon = gtk_image_new();
        gtk_box_pack_start(GTK_BOX(labelBox), icon, FALSE, FALSE, 0);

    // Make the eventbox fill to all left-over space.
    gtk_box_pack_start(GTK_BOX(labelBox), GTK_WIDGET(eventBox), TRUE, TRUE, 0);

    label = GTK_LABEL(gtk_label_new(text.c_str()));
    gtk_container_add(GTK_CONTAINER(eventBox), GTK_WIDGET(label));

    // Align text to the left (x = 0) and in the vertical center (0.5)
    gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);

    closeButton = gtk_button_new();
    gtk_button_set_relief(GTK_BUTTON(closeButton), GTK_RELIEF_NONE);
    gtk_button_set_focus_on_click(GTK_BUTTON(closeButton), FALSE);

#if !GTK_CHECK_VERSION(3, 0, 0)
    // Shrink the padding around the close button
    GtkRcStyle *rcstyle = gtk_rc_style_new();
    rcstyle->xthickness = rcstyle->ythickness = 0;
    gtk_widget_modify_style(closeButton, rcstyle);
    g_object_unref(rcstyle);
#endif

    // Add the stock icon to the close button
    GtkWidget *image = gtk_image_new_from_stock(GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU);
    gtk_container_add(GTK_CONTAINER(closeButton), image);
    gtk_box_pack_start(GTK_BOX(labelBox), closeButton, FALSE, FALSE, 0);

    gtk_widget_set_tooltip_text(closeButton, _("Close tab"));
    gtk_widget_show_all(labelBox);

    tabMenuItem = gtk_radio_menu_item_new_with_label(group, text.c_str());
    group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(tabMenuItem));

    setLabel_gui(text);
    setIcon_gui(type);

    // Associates entry to the widget for later retrieval in MainWindow::switchPage_gui()
    g_object_set_data(G_OBJECT(getContainer()), "entry", (gpointer)this);
}

GtkWidget* BookEntry::getContainer()
{
    return getWidget("mainBox");
}

void BookEntry::setIcon_gui(const EntryType type)
{
    string stock;
    switch (type)
    {
        case Entry::FAVORITE_HUBS : stock = WGETS("icon-favorite-hubs"); break;
        case Entry::FAVORITE_USERS : stock = WGETS("icon-favorite-users"); break;
        case Entry::PUBLIC_HUBS : stock = WGETS("icon-public-hubs"); break;
        case Entry::DOWNLOAD_QUEUE : stock = WGETS("icon-queue"); break;
        case Entry::SEARCH : stock = WGETS("icon-search"); break;
        case Entry::SEARCH_ADL : stock = WGETS("icon-search-adl"); break;
        case Entry::SEARCH_SPY : stock = WGETS("icon-search-spy"); break;
        case Entry::FINISHED_DOWNLOADS : stock = WGETS("icon-finished-downloads"); break;
        case Entry::FINISHED_UPLOADS : stock = WGETS("icon-finished-uploads"); break;
        case Entry::PRIVATE_MESSAGE : stock = WGETS("icon-pm-online"); break;
        case Entry::HUB : stock = WGETS("icon-hub-offline"); break;
        case Entry::SHARE_BROWSER : stock = WGETS("icon-directory"); break;
        default: ; // Default to empty string to indicate no icon should be shown below
    }
    // If user doesn't have the icon in their theme, default to showing no icon instead
    // of showing some generic missing icon. This may occur if the user's system
    // doesn't implement the full freedesktop.org Icon Naming Specification.
    GtkIconTheme *iconTheme = gtk_icon_theme_get_default();
    if (!stock.empty() && gtk_icon_theme_has_icon(iconTheme, stock.c_str()))
        gtk_image_set_from_icon_name(GTK_IMAGE(icon), stock.c_str(), GTK_ICON_SIZE_BUTTON);
}

void BookEntry::setIcon_gui(const std::string stock)
{
    GtkIconTheme *iconTheme = gtk_icon_theme_get_default();
    if (!stock.empty() && gtk_icon_theme_has_icon(iconTheme, stock.c_str()))
        gtk_image_set_from_icon_name(GTK_IMAGE(icon), stock.c_str(), GTK_ICON_SIZE_BUTTON);
}

void BookEntry::setLabel_gui(string text)
{
    // Update the tab menu item label
    GtkWidget *child = gtk_bin_get_child(GTK_BIN(tabMenuItem));
    if (child && GTK_IS_LABEL(child))
        gtk_label_set_text(GTK_LABEL(child), text.c_str());

    // Update the notebook tab label
    gtk_widget_set_tooltip_text(eventBox, text.c_str());
    glong len = g_utf8_strlen(text.c_str(), -1);

    // Truncate the label text
    if (len > labelSize)
    {
        gchar truncatedText[text.size()];
        const string clipText = "...";
        len = labelSize - g_utf8_strlen(clipText.c_str(), -1);
        g_utf8_strncpy(truncatedText, text.c_str(), len);
        truncatedLabelText = truncatedText + clipText;
    }
    else
    {
        truncatedLabelText = text;
    }

    labelText = text;
    updateLabel_gui();

    // Update the main window title if the current tab is selected.
    if (isActive_gui())
        WulforManager::get()->getMainWindow()->setTitle(getLabelText());
}

void BookEntry::setBold_gui()
{
    if (!bold && !isActive_gui())
    {
        bold = TRUE;
        updateLabel_gui();
    }
}

void BookEntry::setUrgent_gui()
{
    if (!isActive_gui())
    {
        MainWindow *mw = WulforManager::get()->getMainWindow();

        if (!urgent)
        {
            bold = TRUE;
            urgent = TRUE;
            updateLabel_gui();
        }

        if (!mw->isActive_gui())
            mw->setUrgent_gui();
    }
}

void BookEntry::setActive_gui()
{
    if (bold || urgent)
    {
        bold = FALSE;
        urgent = FALSE;
        updateLabel_gui();
    }
}

bool BookEntry::isActive_gui()
{
    MainWindow *mw = WulforManager::get()->getMainWindow();

    return mw->isActive_gui() && mw->currentPage_gui() == getContainer();
}

void BookEntry::updateLabel_gui()
{
    const char *format = "%s";

    if (urgent)
        format = "<i><b>%s</b></i>";
    else if (bold)
        format = "<b>%s</b>";

    char *markup = g_markup_printf_escaped(format, truncatedLabelText.c_str());
    gtk_label_set_markup(label, markup);
    g_free(markup);
#if GTK_CHECK_VERSION(3, 0, 0)
    gtk_widget_queue_draw (GTK_WIDGET (label));
#endif
}

const string& BookEntry::getLabelText()
{
    return labelText;
}
