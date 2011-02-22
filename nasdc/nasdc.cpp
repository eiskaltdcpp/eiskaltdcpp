/***************************************************************************
*                                                                         *
*   Copyright (C) 2009-2010  Alexandr Tkachev <tka4ev@gmail.com>          *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "stdafx.h"

#include "dcpp/Util.h"
#include "dcpp/File.h"
#include "dcpp/Thread.h"

#include "utility.h"
#include "ServerManager.h"

#include "VersionGlobal.h"

#ifndef _WIN32
#include <syslog.h>
#include <signal.h>
#endif

#ifdef CLI_DAEMON
#include <readline/readline.h>
#include <readline/history.h>
#endif

static void  logging(bool b, string msg){
    #ifndef _WIN32
        if (b) syslog(LOG_USER | LOG_INFO, msg.c_str());
        else  syslog(LOG_USER | LOG_ERR, msg.c_str());
    #else
        Log(msg);
    #endif
}
#ifndef _WIN32
static void SigHandler(int sig) {
    string str = "Received signal ";

    if (sig == SIGINT) {
        str += "SIGINT";
    } else if (sig == SIGTERM) {
        str += "SIGTERM";
    } else if (sig == SIGQUIT) {
        str += "SIGQUIT";
    } else if (sig == SIGHUP) {
        str += "SIGHUP";
    } else {
        str += Util::toString(sig);
    }

    str += " ending...\n";
    fprintf(stdout,"%s",str.c_str());
    fflush(stdout);
    if (!bDaemon) {
        Log(str);
    } else {
        logging(true, str);
    }

    bIsClose = true;
    ServerStop();

    // restore to default...
    struct sigaction sigact;
    sigact.sa_handler = SIG_DFL;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;

    sigaction(sig, &sigact, NULL);
}
#endif
void printHelp() {
    printf("Using:\n"
           "  eiskaltdcpp-daemon -d\t Run program as daemon\n"
           "  eiskaltdcpp-daemon <Key>\n"
           "EiskaltDC++ is a cross-platform program that uses the Direct Connect and ADC protocol.\n"
           "\n"
           "Keys:\n"
           "  -h, --help\t Show this message\n"
           "  -v, --version\t Show version string\n"
           );
}

void printVersion() {
    printf("%s (%s)\n", EISKALTDCPP_VERSION, EISKALTDCPP_VERSION_SFX);
}

int main(int argc, char* argv[])
{
    for (int i = 0; i < argc; i++){
        if (strcasecmp(argv[i], "-d") == 0) {
            //#ifdef _WIN32
                //bService=true;
            //#else
            #ifndef _WIN32
                bDaemon = true;
            #endif
        }
        else if (!strcmp(argv[i],"--help") || !strcmp(argv[i],"-h")){
            printHelp();
            exit(0);
        }
        else if (!strcmp(argv[i],"--version") || !strcmp(argv[i],"-v")){
            printVersion();
            exit(0);
        }
    }

    sTitle = "eiskaltdcpp-daemon [nasdc] (EiskaltDC++ core 2.2)";

#ifdef _DEBUG
    sTitle += " [debug]";
#endif

    Util::initialize();

    PATH = Util::getPath(Util::PATH_USER_CONFIG);
    #ifndef _WIN32
    if (bDaemon) {
        printf(("Starting "+sTitle+" as daemon using "+PATH+" as config directory.\n").c_str());

        pid_t pid1 = fork();
        if (pid1 == -1) {
            logging(false,"First fork failed!\n");
            return EXIT_FAILURE;
        } else if (pid1 > 0) {
            return EXIT_SUCCESS;
        }

    	if (setsid() == -1) {
            logging(false, "Setsid failed!\n");
            return EXIT_FAILURE;
    	}

        pid_t pid2 = fork();
        if (pid2 == -1) {
            logging(false, "Second fork failed!\n");
            return EXIT_FAILURE;
        } else if (pid2 > 0) {
            return EXIT_SUCCESS;
        }

        chdir("/");

        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);

        umask(117);

        if (open("/dev/null", O_RDWR) == -1) {
            logging(false,  "Failed to open /dev/null!\n");
            return EXIT_FAILURE;
        }

        dup(0);
        dup(0);

        logging(true,  "EiskaltDC++ daemon starting...\n");
    //#else
    //if (bService) {
    //#endif
    } else {
    #endif
        printf(("Starting "+sTitle+" using "+PATH+" as config directory.\n").c_str());
    #ifndef _WIN32
    }
    sigset_t sst;
    sigemptyset(&sst);
    sigaddset(&sst, SIGPIPE);
    sigaddset(&sst, SIGURG);
    sigaddset(&sst, SIGALRM);
//    sigaddset(&sst, SIGSCRTMR);
//    sigaddset(&sst, SIGREGTMR);

    if (bDaemon)
        sigaddset(&sst, SIGHUP);

    pthread_sigmask(SIG_BLOCK, &sst, NULL);

    struct sigaction sigact;

    sigact.sa_handler = SigHandler;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;

    if (sigaction(SIGINT, &sigact, NULL) == -1) {
        printf("Cannot create sigaction SIGINT! %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (sigaction(SIGTERM, &sigact, NULL) == -1) {
        printf("Cannot create sigaction SIGTERM! %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (sigaction(SIGQUIT, &sigact, NULL) == -1) {
        printf("Cannot create sigaction SIGQUIT! ", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (!bDaemon && sigaction(SIGHUP, &sigact, NULL) == -1) {
        printf("Cannot create sigaction SIGHUP! ", strerror(errno));
        exit(EXIT_FAILURE);
    }
    #endif
    ServerInitialize();

    if (!ServerStart()) {
        #ifndef _WIN32
        if (!bDaemon) {
            printf("Server start failed!\n");
        } else {
            logging(false, "Server start failed!\n");
        }
        #else
            printf("Server start failed!\n");
        #endif
        return EXIT_FAILURE;
    }
    #ifndef _WIN32
    else if (!bDaemon) {
        printf((sTitle+" running...\n").c_str());
    }
    #else
        printf((sTitle+" running...\n").c_str());
    #endif
#ifdef CLI_DAEMON
    char *temp, *prompt;
    temp = (char *)NULL;
    prompt = "edcppd$ ";
#endif
    while (bServerRunning) {
        Thread::sleep(1);
        #ifdef CLI_DAEMON
        temp = readline (prompt);

        /* If there is anything on the line, print it and remember it. */
        if (*temp)
        {
            fprintf (stderr, "%s\r\n", temp);
            add_history (temp);
        }

        /* Check for `command' that we handle. */
        if (strcmp (temp, "quit") == 0)
            bServerTerminated = true;

        if (strcmp (temp, "list") == 0)
        {
            HIST_ENTRY **list;
            register int i;

            list = history_list ();
            if (list)
            {
                for (i = 0; list[i]; i++)
                fprintf (stderr, "%d: %s\r\n", i, list[i]->line);
            }
        }
        free (temp);
        #endif
        if (bServerTerminated)
            ServerStop();
    }

    return 0;
}
