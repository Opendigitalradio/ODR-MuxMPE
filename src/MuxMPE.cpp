/*
   Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010,
   2011, 2012 Her Majesty the Queen in Right of Canada (Communications
   Research Center Canada)

   Copyright (C) 2021 Matthias P. Braendli, matthias.braendli@mpb.li

   Copyright (C) 2023 Andy Mace, andy.mace@mediauk.net

    http://www.opendigitalradio.org
*/
/*
   This file is part of ODR-MuxMPE.

   ODR-MuxMPE is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of the
   License, or (at your option) any later version.

   ODR-MuxMPE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with ODR-MuxMPE.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <tsduck.h>
#include "MuxMPEWorker.h"


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "controller/APIController.h"
#include "AppComponent.h"
#include "oatpp-swagger/Controller.hpp"
#include "oatpp/network/Server.hpp"
#include "oatpp/web/server/HttpRequestHandler.hpp"
#include "oatpp/web/protocol/http/incoming/Request.hpp"
#include "oatpp/web/protocol/http/outgoing/ResponseFactory.hpp"

#include <stdlib.h>
#include <memory>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <string>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

// for basename
#include <libgen.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "Log.h"
#include "utils.h"

bool api_at_startup = false;
volatile sig_atomic_t running = 1;
std::shared_ptr<MuxMPEWorker> mpeworker;

/* We are not allowed to use etiLog in the signal handler,
 * because etiLog uses mutexes
 */
void signalHandler(int signum)
{
#ifdef _WIN32
    fprintf(stderr, "\npid: %i\n", _getpid());
#else
    fprintf(stderr, "\npid: %i, ppid: %i\n", getpid(), getppid());
#endif
#define SIG_MSG "Signal received: "
    switch (signum)
    {
#ifndef _WIN32
    case SIGHUP:
        fprintf(stderr, SIG_MSG "SIGHUP\n");
        break;
    case SIGQUIT:
        fprintf(stderr, SIG_MSG "SIGQUIT\n");
        break;
    case SIGPIPE:
        fprintf(stderr, SIG_MSG "SIGPIPE\n");
        return;
        break;
#endif
    case SIGINT:
        fprintf(stderr, SIG_MSG "SIGINT\n");
        fprintf(stderr, "Exiting software\n");
        exit(0);
        break;
    case SIGTERM:
        fprintf(stderr, SIG_MSG "SIGTERM\n");
        fprintf(stderr, "Exiting software\n");
        exit(0);
        break;
    default:
        fprintf(stderr, SIG_MSG "number %i\n", signum);
    }
#ifndef _WIN32
    killpg(0, SIGPIPE);
#endif
    running = 0;
}

void runApiServer()
{

    /* Init oatpp Environment */
    oatpp::base::Environment::init();
    
    /* Register Components in scope of run() method */
    AppComponent components;

    /* Get router component */
    OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);

    auto apiController = std::make_shared<APIController>(mpeworker);

    /* Create APIController and add all of its endpoints to router */
    router->addController(apiController);

    /* Add Swagger UI */
    auto swaggerController = oatpp::swagger::Controller::createShared(apiController->getEndpoints());
    router->addController(swaggerController);

    /* Get connection handler component */
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, connectionHandler);

    /* Get connection provider component */
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider);

    oatpp::network::Server server(connectionProvider, connectionHandler);

    /* Run server */
    server.run();
}

int main(int argc, char *argv[])
{
    // Version handling is done very early to ensure nothing else but the version gets printed out
    if (argc == 2 and strcmp(argv[1], "--version") == 0)
    {
        fprintf(stdout, "%s\n",
#if defined(GITVERSION)
                GITVERSION
#else
                PACKAGE_VERSION
#endif
        );
        return 0;
    }

    header_message();

    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = &signalHandler;

    const int sigs[] = {SIGHUP, SIGQUIT, SIGINT, SIGTERM};
    for (int sig : sigs)
    {
        if (sigaction(sig, &sa, nullptr) == -1)
        {
            perror("sigaction");
            return EXIT_FAILURE;
        }
    }


    etiLog.level(info) << PACKAGE_NAME << " " <<
#if defined(GITVERSION)
        GITVERSION <<
#else
        PACKAGE_VERSION <<
#endif
        " starting up";

    try
    {
        std::string conf_file = "";

        if (argc == 2)
        {
            conf_file = argv[1];

            if (conf_file == "-h")
            {
                printUsage(argv[0], stdout);
                throw InitException("Nothing to do");
            }
        }

        mpeworker = make_shared<MuxMPEWorker>();

        if (conf_file.empty())
        {
            etiLog.level(info) << "No config file specified, waiting for config via API.";
            api_at_startup = true;
        }

        if (!api_at_startup)
        {
            
            //TODO: If its runs as app, we need to honour the logging..

            /* Enable Logging to syslog conditionally */
            //if (pt.get<bool>("general.syslog", false))
            //{
               // etiLog.register_backend(std::make_shared<LogToSyslog>());
            //}

            mpeworker->configureFromFile(conf_file);
            mpeworker->startUpWithConf();
        }
        else
        {
            runApiServer();
        }
    }
    catch (runtime_error &e)
    {
        throw InitException(e.what());
    }
}