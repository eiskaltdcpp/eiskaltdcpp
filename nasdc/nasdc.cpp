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
  #ifdef ENABLE_STACKTRACE
  #include "extra/stacktrace.h"
  #endif
#include <getopt.h>
#include <fstream>
#endif

#ifdef CLI_DAEMON
#include <readline/readline.h>
#include <readline/history.h>
#endif

char pidfile[256] = {0};
char config_dir[1024] = {0};

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

// code of this function based on function tr_daemon from transmission daemon

#if defined(WIN32)
#define USE_NO_DAEMON
#elif !defined(HAVE_DAEMON) || defined(__UCLIBC__)
#define USE_EIDCPP_DAEMON
#else
#define USE_OS_DAEMON
#endif

static int eidcpp_daemon( int nochdir, int noclose ) {
#if defined (USE_OS_DAEMON)
    return daemon (nochdir, noclose);
#elif defined (USE_EIDCPP_DAEMON)
    switch (fork()) {
        case -1: return -1;
        case 0: break;
        default: _exit(0);
    }
    if( setsid( ) == -1 )
        return -1;
    if( !nochdir )
        chdir( "/" );
    if( !noclose ) {
        int fd = open( "/dev/null", O_RDWR, 0 );
        dup2( fd, STDIN_FILENO );
        dup2( fd, STDOUT_FILENO );
        dup2( fd, STDERR_FILENO );
        close( fd );
    }
    return 0;
#else /* USE_NO_DAEMON */
    return 0;
#endif
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
#ifndef _WIN32
           "  -p file, --pidfile=file\t Write daemon process ID to file\n"
#endif // _WIN32
           );
}

void printVersion() {
    printf("%s (%s)\n", EISKALTDCPP_VERSION, EISKALTDCPP_VERSION_SFX);
}

void writePidFile(char *path)
{
	std::ofstream pidfile(path);
	pidfile << getpid();
}

#ifndef _WIN32
static struct option opts[] = {
    { "help",    no_argument,       NULL, 'h'},
    { "version", no_argument,       NULL, 'v'},
    { "daemon",  no_argument,       NULL, 'd'},
    { "confdir", required_argument, NULL, 'c'},
    { "pidfile", required_argument, NULL, 'p'},
    { NULL,      0,                 NULL, 0}
};

void parseArgs(int argc, char* argv[]) {
    int ch;
    while((ch = getopt_long(argc, argv, "hp:c:vd", opts, NULL)) != -1) {
        switch (ch) {
            case 'd':
                bDaemon = true;
                break;
            case 'p':
                strncpy(pidfile, optarg, 256);
                break;
            case 'c':
                strncpy(config_dir, optarg, 1024);
                break;
            case 'v':
                printVersion();
                exit(0);
            case 'h':
                printHelp();
                exit(0);
            default:
                ;
        }
    }
}
#else // _WIN32
void parseArgs(int argc, char* argv[]) {
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
}
#endif // _WIN32

int main(int argc, char* argv[])
{
	parseArgs(argc, argv);

    sTitle = "eiskaltdcpp-daemon [nasdc] (EiskaltDC++ core 2.2)";

#ifdef _DEBUG
    sTitle += " [debug]";
#endif

    Util::PathsMap override;
    if (config_dir[0] != 0)
        override[Util::PATH_USER_CONFIG] = config_dir;
    Util::initialize(override);

    PATH = Util::getPath(Util::PATH_USER_CONFIG);
#ifndef _WIN32
    if (bDaemon) {
        printf(("Starting "+sTitle+" as daemon using "+PATH+" as config directory.\n").c_str());

        if (eidcpp_daemon(true,false) == -1)
            return EXIT_FAILURE;

        umask(117);

        if (pidfile[0] != 0)
            writePidFile(pidfile);

        logging(true,  "EiskaltDC++ daemon starting...\n");
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

#if !defined(_WIN32) && defined(ENABLE_STACKTRACE)
    signal(SIGSEGV, printBacktrace);
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
