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

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <glade/glade.h>

#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>
#include "bacon-message-connection.h"
#include "settingsmanager.hh"
#include "wulformanager.hh"
#include "WulforUtil.hh"
#include <iostream>
#include <signal.h>

#define GUI_LOCALE_DIR LOCALE_DIR

#define GUI_PACKAGE "eiskaltdcpp-gtk"

#include "VersionGlobal.h"

void printHelp()
{
    printf(_(
            "Using:\n"
            "  eiskaltdcpp-gtk <magnet link> <dchub://link> <adc(s)://link>\n"
            "  eiskaltdcpp-gtk <Key>\n"
            "EiskaltDC++ is a cross-platform program that uses the Direct Connect and ADC protocol.\n"
            "\n"
            "Keys:\n"
            "  -h, --help\t Show this message\n"
            "  -v, --version\t Show version string\n"
            )
           );
}

void printVersion()
{
    printf("%s (%s)\n", EISKALTDCPP_VERSION, EISKALTDCPP_VERSION_SFX);
}

BaconMessageConnection *connection = NULL;

void receiver(const char *link, gpointer data)
{
    g_return_if_fail(link != NULL);
    WulforManager::get()->onReceived_gui(link);
}

void callBack(void* x, const std::string& a)
{
    std::cout << _("Loading: ") << a << std::endl;
}

int main(int argc, char *argv[])
{

    for (int i = 0; i < argc; i++){
        if (!strcmp(argv[i],"--help") || !strcmp(argv[i],"-h")){
            printHelp();
            exit(0);
    }
            else if (!strcmp(argv[i],"--version") || !strcmp(argv[i],"-v")){
                printVersion();
                exit(0);
            }
    }

    // Initialize i18n support
    bindtextdomain(GUI_PACKAGE, GUI_LOCALE_DIR);
    textdomain(GUI_PACKAGE);
    bind_textdomain_codeset(GUI_PACKAGE, "UTF-8");

    connection = bacon_message_connection_new(GUI_PACKAGE);

    if (connection != NULL)
        dcdebug("eiskaltdcpp-gtk: connection yes...\n");
    else
        dcdebug("eiskaltdcpp-gtk: connection no...\n");

    // Check if profile is locked
    if (WulforUtil::profileIsLocked())
    {
        if (!bacon_message_connection_get_is_server(connection))
        {
            dcdebug("eiskaltdcpp-gtk: is client...\n");

            if (argc > 1)
            {
                dcdebug("eiskaltdcpp-gtk: send %s\n", argv[1]);
                bacon_message_connection_send(connection, argv[1]);
            }
        }

        bacon_message_connection_free(connection);

        return 0;
    }

    if (bacon_message_connection_get_is_server(connection))
    {
        dcdebug("eiskaltdcpp-gtk: is server...\n");
        bacon_message_connection_set_callback(connection, receiver, NULL);
    }

    // Start the DC++ client core
    dcpp::startup(callBack, NULL);

    dcpp::TimerManager::getInstance()->start();

    g_thread_init(NULL);
    gdk_threads_init();
    gtk_init(&argc, &argv);
    glade_init();
    g_set_application_name("EiskaltDC++ Gtk");

    signal(SIGPIPE, SIG_IGN);

    WulforSettingsManager::newInstance();
    WulforManager::start(argc, argv);
    gdk_threads_enter();
    gtk_main();
    bacon_message_connection_free(connection);
    gdk_threads_leave();
    WulforManager::stop();
    WulforSettingsManager::deleteInstance();

    std::cout << _("Shutting down libdcpp...") << std::endl;
    dcpp::shutdown();
    std::cout << _("Quit...") << std::endl;
    return 0;
}
