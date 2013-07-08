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

#pragma once

#include "entry.hh"

class BookEntry : public Entry
{
    public:
        BookEntry(const EntryType type = EntryType::NONE, const std::string &text = "", const std::string &ui = "", const std::string &id = "");
        virtual ~BookEntry() { }

        GtkWidget *getContainer();
        GtkWidget *getLabelBox() { return labelBox; }
        GtkWidget *getCloseButton() { return closeButton; }
        GtkWidget *getTabMenuItem() { return tabMenuItem; }
        void setIcon_gui(const EntryType type);
        void setIcon_gui(const std::string &stock);
        void setLabel_gui(std::string text);
        const std::string& getLabelText();
        void setBold_gui();
        void setUrgent_gui();
        void setActive_gui();
        bool isActive_gui();
        virtual void show() = 0;

    private:
        void updateLabel_gui();

        std::string labelText;
        std::string truncatedLabelText;
        GtkWidget *eventBox;
        GtkWidget *labelBox;
        GtkWidget *tabMenuItem;
        GtkWidget *closeButton;
        GtkLabel *label;

        bool bold;
        bool urgent;
        static const glong labelSize = 20; ///@todo: make a preference?
        GtkWidget *icon;
};
