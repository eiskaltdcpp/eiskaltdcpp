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

// Description : Hello World in C++, Ansi-style

#include "stdafx.h"

#include "dcpp/Util.h"
#include "dcpp/File.h"
#include "dcpp/Text.h"
#include "dcpp/ResourceManager.h"
#include "dcpp/Exception.h"

#include "utility.h"
#include "ServerManager.h"

#include "VersionGlobal.h"

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
    	AppendSpecialLog(str);
    } else {
    	syslog(LOG_USER | LOG_INFO, str.c_str());
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
        if (!strcmp(argv[i],"--help") || !strcmp(argv[i],"-h")){
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

    for (int i = 0; i < argc; i++) {
        if (strcasecmp(argv[i], "-d") == 0) {
    		bDaemon = true;
    	} else if (strcasecmp(argv[i], "-v") == 0) {
        	printf((sTitle+" built on "+__DATE__+" "+__TIME__+"\n").c_str());
        	return EXIT_SUCCESS;
        } else if (strcasecmp(argv[i], "-h") == 0) {
        	printf("nasdc [-d] [-v]\n");
        	return EXIT_SUCCESS;
        }
    }

    Util::initialize();

    PATH = Util::getPath(Util::PATH_USER_CONFIG);

    if (bDaemon) {
        printf(("Starting "+sTitle+" as daemon using "+PATH+" as config directory.\n").c_str());

        pid_t pid1 = fork();
        if (pid1 == -1) {
            syslog(LOG_USER | LOG_ERR, "First fork failed!\n");
            return EXIT_FAILURE;
        } else if (pid1 > 0) {
            return EXIT_SUCCESS;
        }

    	if (setsid() == -1) {
            syslog(LOG_USER | LOG_ERR, "Setsid failed!\n");
            return EXIT_FAILURE;
    	}

        pid_t pid2 = fork();
        if (pid2 == -1) {
            syslog(LOG_USER | LOG_ERR, "Second fork failed!\n");
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
            syslog(LOG_USER | LOG_ERR, "Failed to open /dev/null!\n");
            return EXIT_FAILURE;
        }

    	dup(0);
        dup(0);

        syslog(LOG_USER | LOG_INFO, "dc++ daemon starting...\n");
    } else {
    	printf(("Starting "+sTitle+" using "+PATH+" as config directory.\n").c_str());
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

	ServerInitialize();

    if (!ServerStart()) {
        if (!bDaemon) {
            printf("Server start failed!\n");
        } else {
            syslog(LOG_USER | LOG_ERR, "Server start failed!\n");
        }
        return EXIT_FAILURE;
    } else if (!bDaemon) {
        printf((sTitle+" running...\n").c_str());
    }
    uint64_t t=0;

	while (bServerRunning) {
		usleep(1000);
        if (bServerTerminated)
            ServerStop();
//#ifdef _DEBUG
       // while (1) {t++;if (t>=1000000000){ ServerStop();}}
//#endif
	}

	return 0;
}
