/*
 * Copyright © 2009 Leliksan Floyd <leliksan@Quadrafon2>
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

/* patch changelog:
 * [05.11.09] модификация исходного кода, убрал лишний код.
 * [05.11.09] масштабирование изображения иконки.
 * [06.11.09] исправлена ошибка, "забивание" экрана уведомлениями, когда период сообщений меньше периода уведомления.
 * [07.11.09] установка уровня уведомления в зависимости от типа сообщения (critical-ошибки, normal-все остальные).
 * [08.11.09] исправлена ошибка, после выхода не закрывалось уведомление.
 * [08.11.09] исправлена ошибка, не обновлялась иконка.
 *
 * Copyright © 2009-2010, author patch: troll, freedcpp, http://code.google.com/p/freedcpp
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "wulformanager.hh"
#include "WulforUtil.hh"
#include "settingsmanager.hh"
#include <dcpp/Text.h>
#include "notify.hh"

#ifndef NOTIFY_CHECK_VERSION
#define NOTIFY_CHECK_VERSION(x,y,z)  0
#endif

using namespace std;
using namespace dcpp;

Notify *Notify::notify = NULL;

void Notify::start()
{
    dcassert(!notify);
    notify = new Notify();
}

void Notify::stop()
{
    dcassert(notify);
    delete notify;
    notify = NULL;
}

Notify* Notify::get()
{
    dcassert(notify);
    return notify;
}

void Notify::init()
{
#ifdef USE_LIBNOTIFY
    notify_init(g_get_application_name());
    notification = notify_notification_new("template", "template", NULL
#if NOTIFY_CHECK_VERSION (0, 7, 0)
    );
#else
    , NULL);
#endif
#endif // USE_LIBNOTIFY
    action = FALSE;
}

void Notify::finalize()
{
#ifdef USE_LIBNOTIFY
    notify_notification_close(notification, NULL);
    g_object_unref(notification);
    notify_uninit();
#endif // USE_LIBNOTIFY
}

void Notify::setCurrIconSize(const int size)
{
    currIconSize = size;

    switch (size)
    {
        case x16:
            icon_width = icon_height = 16; // 16x16
            break;

        case x22:
            icon_width = icon_height = 22; // 22x22
            break;

        case x24:
            icon_width = icon_height = 24; // 24x24
            break;

        case x32:
            icon_width = icon_height = 32; // 32x32
            break;

        case x36:
            icon_width = icon_height = 36; // 36x36
            break;

        case x48:
            icon_width = icon_height = 48; // 48x48
            break;

        case x64:
            icon_width = icon_height = 64; // 64x64
            break;

        case DEFAULT:
            currIconSize = DEFAULT;
            break;

        default:
            currIconSize = DEFAULT;
            WSET("notify-icon-size", DEFAULT);
    }
}

void Notify::showNotify(const string &head, const string &body, TypeNotify notify)
{
    WulforSettingsManager *wsm = WulforSettingsManager::getInstance();

    switch (notify)
    {
        case DOWNLOAD_FINISHED:

            if (action)
            {
#ifdef USE_LIBNOTIFY
                notify_notification_clear_actions(notification);
#endif // USE_LIBNOTIFY
                action = FALSE;
            }

            if (wsm->getInt("notify-download-finished-use"))
            {
#ifdef USE_LIBNOTIFY
                notify_notification_add_action(notification, "1", _("Open file"),
                    (NotifyActionCallback) onAction, g_strdup(body.c_str()), g_free);

                notify_notification_add_action(notification, "2", _("Open folder"),
                    (NotifyActionCallback) onAction, g_strdup(Util::getFilePath(body).c_str()), g_free);
#endif // USE_LIBNOTIFY

                showNotify(wsm->getString("notify-download-finished-title"), head, Util::getFileName(body),
                    wsm->getString("notify-download-finished-icon"), wsm->getInt("notify-icon-size"), NOTIFY_URGENCY_NORMAL);

                action = TRUE;
            }

            break;

        case DOWNLOAD_FINISHED_USER_LIST:

            if (wsm->getInt("notify-download-finished-ul-use"))
            showNotify(wsm->getString("notify-download-finished-ul-title"), head, body,
                wsm->getString("notify-download-finished-ul-icon"), wsm->getInt("notify-icon-size"), NOTIFY_URGENCY_LOW);
            break;

        case PRIVATE_MESSAGE:

            if (wsm->getInt("notify-private-message-use"))
            showNotify(wsm->getString("notify-private-message-title"), head, body,
                wsm->getString("notify-private-message-icon"), wsm->getInt("notify-icon-size"), NOTIFY_URGENCY_NORMAL);
            break;

        case HUB_CONNECT:

            if (wsm->getInt("notify-hub-connect-use"))
            showNotify(wsm->getString("notify-hub-connect-title"), head, body,
                wsm->getString("notify-hub-connect-icon"), wsm->getInt("notify-icon-size"), NOTIFY_URGENCY_NORMAL);
            break;

        case HUB_DISCONNECT:

            if (wsm->getInt("notify-hub-disconnect-use"))
            showNotify(wsm->getString("notify-hub-disconnect-title"), head, body,
                wsm->getString("notify-hub-disconnect-icon"), wsm->getInt("notify-icon-size"), NOTIFY_URGENCY_CRITICAL);
            break;

        case FAVORITE_USER_JOIN:

            if (wsm->getInt("notify-fuser-join"))
            showNotify(wsm->getString("notify-fuser-join-title"), head, body,
                wsm->getString("notify-fuser-join-icon"), wsm->getInt("notify-icon-size"), NOTIFY_URGENCY_NORMAL);
            break;

        case FAVORITE_USER_QUIT:

            if (wsm->getInt("notify-fuser-quit"))
            showNotify(wsm->getString("notify-fuser-quit-title"), head, body,
                wsm->getString("notify-fuser-quit-icon"), wsm->getInt("notify-icon-size"), NOTIFY_URGENCY_NORMAL);
            break;

        default: break;
    }
}

void Notify::showNotify(const string &title, const string &head, const string &body, const string &icon, const int iconSize, NotifyUrgency urgency)
{
    if (title.empty())
        return;

    gchar *esc_title = g_markup_escape_text(title.c_str(), -1);
    gchar *esc_body = g_markup_escape_text(body.c_str(), -1);
    string message = head + esc_body;

#ifdef USE_LIBNOTIFY
    notify_notification_close(notification, NULL);
    notify_notification_clear_hints(notification);
    notify_notification_update(notification, esc_title, message.c_str(), NULL);
    notify_notification_set_urgency(notification, urgency);
#endif // USE_LIBNOTIFY

    g_free(esc_title);
    g_free(esc_body);

    if (!icon.empty())
    {
        setCurrIconSize(iconSize);
        GdkPixbuf *pixbuf = NULL;

        if (currIconSize != DEFAULT)
        {
            GdkPixbuf *temp = gdk_pixbuf_new_from_file(Text::fromUtf8(icon).c_str(), NULL);

            if (temp)
            {
                pixbuf = WulforUtil::scalePixbuf(temp, icon_width, icon_height);
                g_object_unref(temp);
            }
        }
        else
        {
            pixbuf = gdk_pixbuf_new_from_file(Text::fromUtf8(icon).c_str(), NULL);
        }

        if (pixbuf)
        {
#ifdef USE_LIBNOTIFY
            notify_notification_set_icon_from_pixbuf(notification, pixbuf);
#endif // USE_LIBNOTIFY
            g_object_unref(pixbuf);
        }
    }

    if (action)
    {
#ifdef USE_LIBNOTIFY
        notify_notification_clear_actions(notification);
#endif // USE_LIBNOTIFY
        action = FALSE;
    }

#ifdef USE_LIBNOTIFY
    notify_notification_show(notification, NULL);
#endif // USE_LIBNOTIFY
}

void Notify::onAction(NotifyNotification *notify, const char *action, gpointer data)
{
    string target = (gchar*)data;

    if (!target.empty())
        WulforUtil::openURI(target);

#ifdef USE_LIBNOTIFY
    notify_notification_close(notify, NULL);
#endif // USE_LIBNOTIFY
}
