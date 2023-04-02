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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

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

#include <iterator>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <functional>
#include <algorithm>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "Log.h"
#include "utils.h"


using namespace std;
using boost::property_tree::ptree;

volatile sig_atomic_t running = 1;

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

#ifdef _WIN32
    if (SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST) == 0)
    {
        etiLog.log(warn, "Can't increase priority: %s\n", strerror(errno));
    }
#else
    // Use the lowest real-time priority for this thread, and switch to real-time scheduling
    const int policy = SCHED_RR;
    sched_param sp;
    sp.sched_priority = sched_get_priority_min(policy);
    int thread_prio_ret = pthread_setschedparam(pthread_self(), policy, &sp);
    if (thread_prio_ret != 0)
    {
        etiLog.level(error) << "Could not set real-time priority for thread:" << thread_prio_ret;
    }
#endif

    int returnCode = 0;
    ptree pt;

    try
    {
        string conf_file = "";

        if (argc == 2) {
            conf_file = argv[1];

            if (conf_file == "-h")
            {
                printUsage(argv[0], stdout);
                throw InitException("Nothing to do");
            }
        }

        if (conf_file.empty())
        {
            printUsage(argv[0], stderr);
            throw InitException("No configuration file specified");
        }

        try
        {
            if (stringEndsWith(conf_file, ".json"))
            {
                read_json(conf_file, pt);
            }
            else
            {
                read_info(conf_file, pt);
            }
        }
        catch (runtime_error &e)
        {
            throw InitException(e.what());
        }

        /* Enable Logging to syslog conditionally */
        if (pt.get<bool>("general.syslog", false))
        {
            etiLog.register_backend(std::make_shared<LogToSyslog>());
        }

        const auto startupcheck = pt.get<string>("general.startupcheck", "");
        if (not startupcheck.empty())
        {
            etiLog.level(info) << "Running startup check '" << startupcheck << "'";
            int wstatus = system(startupcheck.c_str());

            if (WIFEXITED(wstatus))
            {
                if (WEXITSTATUS(wstatus) == 0)
                {
                    etiLog.level(info) << "Startup check ok";
                }
                else
                {
                    etiLog.level(error) << "Startup check failed, returned " << WEXITSTATUS(wstatus);
                    return 1;
                }
            }
            else
            {
                etiLog.level(error) << "Startup check failed, child didn't terminate normally";
                return 1;
            }
        }

        etiLog.level(info) << PACKAGE_NAME << " " <<
#if defined(GITVERSION)
            GITVERSION <<
#else
            PACKAGE_VERSION <<
#endif
            " starting up";
    }
    catch (const InitException &except)
    {
        etiLog.level(error) << "initialisation aborted: " << except.what();
        returnCode = 1;
    }
    catch (const std::invalid_argument &except)
    {
        etiLog.level(error) << "Caught invalid argument : " << except.what();
        returnCode = 1;
    }
    catch (const std::out_of_range &except)
    {
        etiLog.level(error) << "Caught out of range exception : " << except.what();
        returnCode = 1;
    }
    catch (const std::logic_error &except)
    {
        etiLog.level(error) << "Caught logic error : " << except.what();
        returnCode = 2;
    }
    catch (const std::runtime_error &except)
    {
        etiLog.level(error) << "Caught runtime error : " << except.what();
        returnCode = 2;
    }

    if (returnCode == 0)
    {
        std::vector<std::shared_ptr<udp_source_t>> inputs;
        // Setup Inputs
        set<string> all_input_names;
        for (auto pt_input : pt.get_child("inputs"))
        {
            string uid = pt_input.first;

            // check for uniqueness of the uid
            if (all_input_names.count(uid) == 0)
            {
                all_input_names.insert(uid);
            }
            else
            {
                stringstream ss;
                ss << "input with uid " << uid << " not unique!";
                throw runtime_error(ss.str());
            }

            //Build input
            auto input = make_shared<udp_source_t>();
            input->name = pt_input.second.get<std::string>("name");
            input->protocol = pt_input.second.get<std::string>("protocol");
            input->source = pt_input.second.get<std::string>("source");
            input->sourceip = pt_input.second.get<std::string>("sourceip");
            input->sourceport = pt_input.second.get<unsigned int>("sourceport");

            inputs.push_back(input);
        }

        // Setup output.
        ptree pt_output = pt.get_child("output");

        auto dest = make_shared<ts_destination_t>();
        dest->output = pt_output.get<string>("output");
        dest->payload_pid = pt_output.get<unsigned int>("payload_pid");
        dest->pmt_pid = pt_output.get<unsigned int>("pmt_pid");
        dest->ts_id = pt_output.get<unsigned int>("ts_id");
        dest->service_type = pt_output.get<unsigned int>("service_type");
        dest->service_id = pt_output.get<unsigned int>("service_id");
        dest->service_name = pt_output.get<string>("service_name");
        dest->service_provider_name = pt_output.get<string>("service_provider_name");
        dest->output = pt_output.get<string>("output");
        dest->output_host = pt_output.get<string>("output_host");
        dest->output_port = pt_output.get<unsigned int>("output_port");
        dest->output_source_address = pt_output.get<string>("output_source_address");
        dest->output_ttl = pt_output.get<unsigned int>("output_ttl");

        if (dest->output == "srt")
        {
            dest->output_srt_passphrase = pt_output.get<string>("output_srt_passphrase");
        }

        int debuglevel = -1;
        if (pt.get_child("general").get<string>("tsduck_debug") == "true")
        {
            debuglevel = ts::Severity::Debug;
        } else {
            debuglevel = ts::Severity::Info;
        }

        ts::AsyncReport report(debuglevel);
        ts::TSProcessor tsproc(report);

        // Create and start a background system monitor.
        ts::SystemMonitor monitor(report);
        monitor.start();

        // Build tsp options. Accept most default values, except a few ones.
        ts::TSProcessorArgs opt;
        opt.app_name = u"odr-muxmpe"; // for error messages only.
        opt.fixed_bitrate = 5000000;
        opt.max_flush_pkt = 70;

        opt.input = {u"null", {}};

        ts::UString pat = u"<?xml version=\"1.0\" encoding=\"UTF-8\"?><tsduck><PAT version=\"0\" transport_stream_id=\"" + ts::UString::Decimal(dest->ts_id,  0, true, u"", false, ts::SPACE) + u"\" network_PID=\"0x0010\"><service service_id=\"" + ts::UString::Decimal(dest->service_id,  0, true, u"", false, ts::SPACE) + u"\" program_map_PID=\"" + ts::UString::Decimal(dest->pmt_pid,  0, true, u"", false, ts::SPACE) + u"\"/></PAT></tsduck>";
        ts::UString pmt = u"<?xml version=\"1.0\" encoding=\"UTF-8\"?><tsduck><PMT version=\"0\" service_id=\"" + ts::UString::Decimal(dest->service_id,  0, true, u"", false, ts::SPACE) + u"\"><component elementary_PID=\"" + ts::UString::Decimal(dest->payload_pid,  0, true, u"", false, ts::SPACE) + u"\" stream_type=\"" + ts::UString::Decimal(dest->service_type,  0, true, u"", false, ts::SPACE) + u"\"/></PMT></tsduck>";
        ts::UString sdt = u"<?xml version=\"1.0\" encoding=\"UTF-8\"?><tsduck><SDT version=\"0\" current=\"true\" transport_stream_id=\"" + ts::UString::Decimal(dest->service_id,  0, true, u"", false, ts::SPACE) + u"\" original_network_id=\"2\" actual=\"true\"> <service service_id=\"" + ts::UString::Decimal(dest->service_id,  0, true, u"", false, ts::SPACE) + u"\" EIT_schedule=\"false\" EIT_present_following=\"false\" running_status=\"running\" CA_mode=\"false\"><service_descriptor service_type=\"0x0C\" service_name=\"" + ts::UString::FromUTF8(dest->service_name) + u"\" service_provider_name=\"" + ts::UString::FromUTF8(dest->service_provider_name) + u"\"/> </service></SDT></tsduck>";

        opt.plugins = {
            {u"regulate", {u"--packet-burst", u"14"}},
            {u"inject", {pat, u"--pid", u"0", u"-s", u"--inter-packet", u"100"}},
            {u"inject", {pmt, u"--pid", u"256", u"-s", u"--inter-packet", u"100"}},
            {u"inject", {sdt, u"--pid", u"17", u"-s", u"--inter-packet", u"1000"}},
        };

        //MPEInject
        ts::PluginOptions po;
        po.name = u"mpeinject";
        po.args.push_back(u"-b");
        po.args.push_back(u"100000");
        po.args.push_back(u"--max-queue");
        po.args.push_back(u"10000");
        po.args.push_back(u"-p");
        po.args.push_back(ts::UString::Decimal(dest->payload_pid,  0, true, u"", false, ts::SPACE));

        for (auto input : inputs) {
            po.args.push_back(ts::UString::FromUTF8(input->source) +u":"+ ts::UString::Decimal(input->sourceport,  0, true, u"", false, ts::SPACE));
        }

        opt.plugins.push_back(po);

        ts::UString port = ts::UString::Decimal(dest->output_port, 0, true, u"", false, ts::SPACE);
        if (dest->output == "srt")
        {
            if (dest->output_srt_passphrase != "")
            {
                opt.output = {u"srt", {u"-c", ts::UString::FromUTF8(dest->output_host) + u":" + port, u"-e", u"--local-interface", ts::UString::FromUTF8(dest->output_source_address), u"--ipttl", ts::UString::Decimal(dest->output_ttl), u"--passphrase", ts::UString::FromUTF8(dest->output_srt_passphrase)}};
            }
            else
            {
                opt.output = {u"srt", {u"-c", ts::UString::FromUTF8(dest->output_host) + u":" + port, u"-e", u"--local-interface", ts::UString::FromUTF8(dest->output_source_address), u"--ipttl", ts::UString::Decimal(dest->output_ttl)}};
            }
        }
        else if (dest->output == "udp")
        {
            opt.output = {u"ip", {u"-e", u"-f", u"-l", ts::UString::FromUTF8(dest->output_source_address), ts::UString::FromUTF8(dest->output_host) + u":" + port}};
        }
        else
        {
            printf("TS Processing failed to start. TS Output not available\n");
            return -1;
        }

        // Start the TS processing.
        if (!tsproc.start(opt))
        {
            etiLog.log(info, "TS Processing failed to start. TS Output not available\n");
            return -1;
        }

        // And wait for TS processing termination.
        tsproc.waitForTermination();
        etiLog.log(info, "TS Processing Thread Terminated");
    }

    etiLog.log(debug, "exiting...\n");
    fflush(stderr);

    if (returnCode != 0)
    {
        etiLog.log(alert, "...aborting\n");
    }
    else
    {
        etiLog.log(debug, "...done\n");
    }

    return returnCode;
}
