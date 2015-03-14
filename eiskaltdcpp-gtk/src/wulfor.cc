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

#ifdef ENABLE_STACKTRACE
#include "extra/stacktrace.h"
#endif // ENABLE_STACKTRACE

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
            "  -V, --version\t Show version string\n"
            )
           );
}

void printVersion()
{
    printf("%s version: %s (%s)\n", APPNAME, EISKALTDCPP_VERSION, EISKALTDCPP_VERSION_SFX);
    printf("GTK+ version: %d.%d.%d\n", gtk_major_version, gtk_minor_version, gtk_micro_version);
    printf("Glib version: %d.%d.%d\n", glib_major_version, glib_minor_version, glib_micro_version);
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

void catchSIG(int sigNum) {
    psignal(sigNum, _("Catching signal "));

#ifdef ENABLE_STACKTRACE
    printBacktrace(sigNum);
#endif // ENABLE_STACKTRACE

    raise(SIGINT);

    std::abort();
}

template <int sigNum = 0, int ... Params>
void catchSignals() {
    if (!sigNum)
        return;

    psignal(sigNum, _("Installing handler for"));

    signal(sigNum, catchSIG);

    catchSignals<Params ... >();
}

void installHandlers(){
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;

    if (sigaction(SIGPIPE, &sa, NULL) == -1)
        printf(_("Cannot handle SIGPIPE\n"));
    else {
        sigset_t set;
        sigemptyset (&set);
        sigaddset (&set, SIGPIPE);
        pthread_sigmask(SIG_BLOCK, &set, NULL);
    }

    catchSignals<SIGSEGV, SIGABRT, SIGBUS, SIGTERM>();

    printf(_("Signal handlers installed.\n"));
}

int main(int argc, char *argv[])
{

    for (int i = 0; i < argc; i++){
        if (!strcmp(argv[i],"--help") || !strcmp(argv[i],"-h")){
            printHelp();
            exit(0);
    }
            else if (!strcmp(argv[i],"--version") || !strcmp(argv[i],"-V")){
                printVersion();
                exit(0);
            }
    }

    // Initialize i18n support
    bindtextdomain(GUI_PACKAGE, GUI_LOCALE_DIR);
    textdomain(GUI_PACKAGE);
    bind_textdomain_codeset(GUI_PACKAGE, "UTF-8");

    connection = bacon_message_connection_new(GUI_PACKAGE);

    dcdebug(connection ? "eiskaltdcpp-gtk: connection yes...\n" : "eiskaltdcpp-gtk: connection no...\n");

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

#if !GLIB_CHECK_VERSION(2,32,0)
    g_thread_init(NULL);
#endif
    gdk_threads_init();
    gtk_init(&argc, &argv);
    g_set_application_name("EiskaltDC++ Gtk");

    installHandlers();

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
