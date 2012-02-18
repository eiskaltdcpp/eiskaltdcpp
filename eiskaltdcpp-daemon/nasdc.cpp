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

char pidfile[256] = {0};
char config_dir[1024] = {0};
char local_dir[1024] = {0};

static void  logging(bool b, string msg){
#ifndef _WIN32
    if (b) syslog(LOG_USER | LOG_INFO, "%s", msg.c_str());
    else  syslog(LOG_USER | LOG_ERR, "%s", msg.c_str());
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
           "  eiskaltdcpp-daemon -d [options]\n"
           "  eiskaltdcpp-daemon <Key>\n"
           "EiskaltDC++ is a cross-platform program that uses the Direct Connect and ADC protocol.\n"
           "\n"
           "Keys:\n"
           "  -d, --daemon\t Run program as daemon\n"
           "  -h, --help\t Show this message\n"
           "  -v, --version\t Show version string\n"
#ifndef _WIN32
           "  -V, --verbose\t Verbose mode\n"
           "  -D, --debug\t Debug mode\n"
           "  -P <port>, --port=<port>\t Set port for XMLRPC or JSONRPC (default: 3121)\n"
           "  -L <ip>,   --ip=<ip>\t\t Set IP address for XMLRPC or JSONRPC (default: 127.0.0.1)\n"
           "  -p <file>, --pidfile=<file>\t Write daemon process ID to <file>\n"
           "  -c <dir>,  --confdir=<dir>\t Store config in <dir>\n"
           "  -l <dir>,  --localdir=<dir>\t Store local data (cache, temp files) in <dir> (defaults is equal confdir)\n"
#ifdef XMLRPC_DAEMON
           "  -S <file>,  --rpclog=<file>\t Write xmlrpc log to <file> (default: /tmp/eiskaltdcpp-daemon.xmlrpc.log)\n"
           "  -U <uripath>,  --uripath=<uripath>\t Set UriPath for xmlrpc abyss server to <uripath> (default: /eiskaltdcpp)\n"
#endif
#endif // _WIN32
           );
}

void printVersion() {
    printf("%s (%s)\n", EISKALTDCPP_VERSION, EISKALTDCPP_VERSION_SFX);
}

#ifndef _WIN32
static struct option opts[] = {
    { "help",    no_argument,       NULL, 'h'},
    { "version", no_argument,       NULL, 'v'},
    { "daemon",  no_argument,       NULL, 'd'},
    { "verbose", no_argument,       NULL, 'V'},
    { "debug",   no_argument,       NULL, 'D'},
    { "confdir", required_argument, NULL, 'c'},
    { "localdir",required_argument, NULL, 'l'},
    { "pidfile", required_argument, NULL, 'p'},
    { "port",    required_argument, NULL, 'P'},
    { "rpclog",  required_argument, NULL, 'S'},
    { "uripath", required_argument, NULL, 'U'},
    { "ip",      required_argument, NULL, 'L'},
    { NULL,      0,                 NULL, 0}
};

void writePidFile(char *path)
{
    std::ofstream pidfile(path);
    pidfile << getpid();
}

void parseArgs(int argc, char* argv[]) {
    int ch;
    while((ch = getopt_long(argc, argv, "hvdVDp:c:l:P:L:S:", opts, NULL)) != -1) {
        switch (ch) {
            case 'P':
                lport = (unsigned short int) atoi (optarg);
                break;
            case 'L':
                lip.assign(optarg,50);
                break;
            case 'S':
                xmlrpcLog.assign(optarg,1024);
                break;
           case 'U':
                xmlrpcUriPath.assign(optarg,1024);
                break;
            case 'V':
                isVerbose = true;
                break;
            case 'D':
                isDebug = true;
                break;
            case 'd':
                bDaemon = true;
                break;
            case 'p':
                strncpy(pidfile, optarg, 256);
                break;
            case 'c':
                strncpy(config_dir, optarg, 1024);
                break;
            case 'l':
                strncpy(local_dir, optarg, 1024);
                break;
            case 'v':
                printVersion();
                exit(0);
            case 'h':
                printHelp();
                exit(0);
            default:
                exit(0);
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

    sTitle = "eiskaltdcpp-daemon (EiskaltDC++ core 2.2)";

#ifdef _DEBUG
    sTitle += " [debug]";
#endif

    Util::PathsMap override;
    if (config_dir[0] != 0) {
        override[Util::PATH_USER_CONFIG] = config_dir;
        override[Util::PATH_USER_LOCAL] = config_dir;
    }
    if (local_dir[0] != 0)
        override[Util::PATH_USER_LOCAL] = local_dir;
    Util::initialize(override);
    
#ifdef _DEBUG
    printf("PATH_GLOBAL_CONFIG: %s\n\
            PATH_USER_CONFIG: %s\n\
            PATH_USER_LOCAL: %s\n\
            PATH_RESOURCES: %s\n\
            PATH_LOCALE: %s\n\
            PATH_DOWNLOADS: %s\n\
            PATH_FILE_LISTS %s\n\
            PATH_HUB_LISTS: %s\n\
            PATH_NOTEPAD: %s\n",
            Util::getPath(Util::PATH_GLOBAL_CONFIG).c_str(),
            Util::getPath(Util::PATH_USER_CONFIG).c_str(),
            Util::getPath(Util::PATH_USER_LOCAL).c_str(),
            Util::getPath(Util::PATH_RESOURCES).c_str(),
            Util::getPath(Util::PATH_LOCALE).c_str(),
            Util::getPath(Util::PATH_DOWNLOADS).c_str(),
            Util::getPath(Util::PATH_FILE_LISTS).c_str(),
            Util::getPath(Util::PATH_HUB_LISTS).c_str(),
            Util::getPath(Util::PATH_NOTEPAD).c_str()
                                    );
    fflush(stdout);
#endif

    PATH = Util::getPath(Util::PATH_USER_CONFIG);
#ifndef _WIN32
    if (bDaemon) {
        printf("%s\n",("Starting "+sTitle+" as daemon using "+PATH+" as config directory.").c_str());

        if (eidcpp_daemon(true,false) == -1)
            return EXIT_FAILURE;

        //umask(117);

        if (pidfile[0] != 0)
            writePidFile(pidfile);

        logging(true,  "EiskaltDC++ daemon starting...\n");
    } else {
#endif
        printf("%s\n",("Starting "+sTitle+" using "+PATH+" as config directory.").c_str());
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
        printf("Cannot create sigaction SIGQUIT! %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (!bDaemon && sigaction(SIGHUP, &sigact, NULL) == -1) {
        printf("Cannot create sigaction SIGHUP! %s\n", strerror(errno));
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
        printf("%s\n",(sTitle+" running...").c_str());
    }
#else
        printf("%s\n",(sTitle+" running...").c_str());
#endif

#if !defined(_WIN32) && defined(ENABLE_STACKTRACE)
    signal(SIGSEGV, printBacktrace);
#endif

    while (bServerRunning) {
        Thread::sleep(1);
        if (bServerTerminated)
            ServerStop();
    }

    return 0;
}
