#include "MuxMPEWorker.h"
#include <tsduck.h>

MuxMPEWorker::MuxMPEWorker()
{
// Constructor implementation
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
}

MuxMPEWorker::~MuxMPEWorker()
{
    // Destructor implementation*
}

int MuxMPEWorker::createTapInterface(std::string tap_name, std::string tap_ip)
{
    int fd = open("/dev/net/tun", O_RDWR);
    if (fd < 0)
    {
        perror("open");
        return -1;
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    strncpy(ifr.ifr_name, tap_name.c_str(), IFNAMSIZ);

    if (ioctl(fd, TUNSETIFF, (void *)&ifr) < 0)
    {
        perror("ioctl");
        close(fd);
        return -1;
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        close(fd);
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(tap_ip.c_str());
    memcpy(&ifr.ifr_addr, &addr, sizeof(addr));

    if (ioctl(sockfd, SIOCSIFADDR, &ifr) < 0)
    {
        perror("ioctl");
        close(sockfd);
        close(fd);
        return -1;
    }

    if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0)
    {
        perror("ioctl");
        close(sockfd);
        close(fd);
        return -1;
    }

    ifr.ifr_flags |= IFF_UP | IFF_RUNNING;

    if (ioctl(sockfd, SIOCSIFFLAGS, &ifr) < 0)
    {
        perror("ioctl");
        close(sockfd);
        close(fd);
        return -1;
    }

    close(sockfd);
    return fd;
}

void MuxMPEWorker::startUpWithConf()
{
    ts_thread = new std::thread(&MuxMPEWorker::BringUpMux, this);
    ts_thread->join();
}

bool MuxMPEWorker::startup()
{
    if ((haveConfig) && (!isRunning))
    {

        ts_thread = new std::thread(&MuxMPEWorker::BringUpMux, this);

        if (ts_thread)
        {
            isRunning = true;
            ts_thread->detach();
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

bool MuxMPEWorker::stop()
{
    if (tsproc->isStarted())
    {
        tsproc->abort();
        isRunning = false;
        return true;
    }
    else
    {
        return false;
    }
}

void MuxMPEWorker::BringUpMux()
{
    isRunning = true;

    if (unicastinputs.size() > 0)
    {
        use_tap = true;
        // Call create_tap_interface() on the multicast_tap instance
        int tap_fd = createTapInterface(tap_interface, tap_ip);
        if (tap_fd < 0)
        {
            std::cerr << "Failed to create TAP interface" << std::endl;
            return;
        }

        for (auto input : unicastinputs)
        {
            etiLog.log(debug, "Setting up Unicast to Multicast: Interface: %s, Listen Port: %d, Mcast Address:%s, Mcast Port: %d\n", tap_interface.c_str(), input->listenport, input->mcast_address.c_str(), input->mcast_port);

            // Create a shared_ptr of MulticastTap
            std::shared_ptr<MulticastTap> tap = std::make_shared<MulticastTap>(tap_interface, input->mcast_address, input->listenport, input->mcast_port, 65535);

            // Create a thread using a lambda to capture the shared_ptr and run the MulticastTap::run function
            std::thread t([tap]()
                          { tap->run(); });

            t.detach();
        }
    }

    ts::AsyncReport report(debuglevel);
    tsproc = new ts::TSProcessor(report);

    // Create and start a background system monitor.
    ts::SystemMonitor monitor(report);
    monitor.start();

    // Build tsp options. Accept most default values, except a few ones.
    ts::TSProcessorArgs opt;
    opt.app_name = u"odr-muxmpe"; // for error messages only.
    opt.fixed_bitrate = dest->bitrate;
    opt.max_flush_pkt = 70;

    int pcr_distance = dest->bitrate / (5 * 188 * 8);

    opt.input = {u"null", {}};

    ts::UString pat = u"<?xml version=\"1.0\" encoding=\"UTF-8\"?><tsduck><PAT version=\"0\" transport_stream_id=\"" + ts::UString::Decimal(dest->ts_id, 0, true, u"", false, ts::SPACE) + u"\" network_PID=\"0x0010\"><service service_id=\"" + ts::UString::Decimal(dest->service_id, 0, true, u"", false, ts::SPACE) + u"\" program_map_PID=\"" + ts::UString::Decimal(dest->pmt_pid, 0, true, u"", false, ts::SPACE) + u"\"/></PAT></tsduck>";
    ts::UString pmt = u"<?xml version=\"1.0\" encoding=\"UTF-8\"?><tsduck><PMT version=\"0\" service_id=\"" + ts::UString::Decimal(dest->service_id, 0, true, u"", false, ts::SPACE) + u"\" PCR_PID=\"8190\"><component elementary_PID=\"" + ts::UString::Decimal(dest->payload_pid, 0, true, u"", false, ts::SPACE) + u"\" stream_type=\"0x0D\"><stream_identifier_descriptor component_tag=\"1\"/><data_broadcast_id_descriptor data_broadcast_id=\"0x0005\"/></component></PMT></tsduck>";
    ts::UString sdt = u"<?xml version=\"1.0\" encoding=\"UTF-8\"?><tsduck><SDT version=\"0\" current=\"true\" transport_stream_id=\"" + ts::UString::Decimal(dest->service_id, 0, true, u"", false, ts::SPACE) + u"\" original_network_id=\"2\" actual=\"true\"> <service service_id=\"" + ts::UString::Decimal(dest->service_id, 0, true, u"", false, ts::SPACE) + u"\" EIT_schedule=\"false\" EIT_present_following=\"false\" running_status=\"running\" CA_mode=\"false\"><service_descriptor service_type=\"0x0C\" service_name=\"" + ts::UString::FromUTF8(dest->service_name) + u"\" service_provider_name=\"" + ts::UString::FromUTF8(dest->service_provider_name) + u"\"/> </service></SDT></tsduck>";

    opt.plugins = {
        {u"regulate", {u"--packet-burst", u"14"}},
        {u"filter", {u"--every", ts::UString::Decimal(pcr_distance, 0, true, u"", false, ts::SPACE), u"--set-label", u"1"}},
        {u"craft", {u"--only-label", u"1", u"--pid", u"8190", u"--no-payload", u"--pcr", u"0"}},
        {u"continuity", {u"--pid", u"8190", u"--fix"}},
        {u"pcradjust", {u"--pid", u"8190"}},
        {u"inject", {pat, u"--pid", u"0", u"-s", u"--inter-packet", u"100"}},
        {u"inject", {pmt, u"--pid", u"256", u"-s", u"--inter-packet", u"100"}},
        {u"inject", {sdt, u"--pid", u"17", u"-s", u"--inter-packet", u"1000"}},
    };

    // MPEInject
    ts::PluginOptions po;
    po.name = u"mpeinject";

    po.args.push_back(u"-b");
    po.args.push_back(u"100000");
    po.args.push_back(u"--max-queue");
    po.args.push_back(u"100");
    po.args.push_back(u"-p");
    po.args.push_back(ts::UString::Decimal(dest->payload_pid, 0, true, u"", false, ts::SPACE));

    if (use_tap)
    {
        po.args.push_back(u"-l");
        po.args.push_back(ts::UString::FromUTF8(tap_ip).c_str());
    }

    for (auto input : inputs)
    {
        po.args.push_back(ts::UString::FromUTF8(input->source) + u":" + ts::UString::Decimal(input->sourceport, 0, true, u"", false, ts::SPACE));
    }

    opt.plugins.push_back(po);

    // Setup output
    ts::UString port = ts::UString::Decimal(dest->output_port, 0, true, u"", false, ts::SPACE);
    if (dest->output == "srt")
    {
        if (dest->output_srt_passphrase != "")
        {
            if (dest->output_source_address.empty())
            {
                opt.output = {u"srt", {u"-c", ts::UString::FromUTF8(dest->output_host) + u":" + port, u"-e", u"--ipttl", ts::UString::Decimal(dest->output_ttl), u"--passphrase", ts::UString::FromUTF8(dest->output_srt_passphrase)}};
            }
            else
            {
                opt.output = {u"srt", {u"-c", ts::UString::FromUTF8(dest->output_host) + u":" + port, u"-e", u"--local-interface", ts::UString::FromUTF8(dest->output_source_address), u"--ipttl", ts::UString::Decimal(dest->output_ttl), u"--passphrase", ts::UString::FromUTF8(dest->output_srt_passphrase)}};
            }
        }
        else
        {
            if (dest->output_source_address.empty())
            {
                opt.output = {u"srt", {u"-c", ts::UString::FromUTF8(dest->output_host) + u":" + port, u"-e", u"--ipttl", ts::UString::Decimal(dest->output_ttl)}};
            }
            else
            {
                opt.output = {u"srt", {u"-c", ts::UString::FromUTF8(dest->output_host) + u":" + port, u"-e", u"--local-interface", ts::UString::FromUTF8(dest->output_source_address), u"--ipttl", ts::UString::Decimal(dest->output_ttl)}};
            }
        }
    }
    else if (dest->output == "udp")
    {
        if (dest->output_source_address.empty())
        {
            opt.output = {u"ip", {u"-e", u"-f", u"-l", ts::UString::FromUTF8(dest->output_host) + u":" + port}};
        }
        else
        {
            opt.output = {u"ip", {u"-e", u"-f", u"-l", ts::UString::FromUTF8(dest->output_source_address), ts::UString::FromUTF8(dest->output_host) + u":" + port}};
        }
    }
    else
    {
        printf("TS Processing failed to start. TS Output not available\n");
        // return -1;
    }

    // Start the TS processing.
    if (!tsproc->start(opt))
    {
        etiLog.log(info, "TS Processing failed to start. TS Output not available\n");
    }

    // And wait for TS processing termination.
    tsproc->waitForTermination();
    etiLog.log(info, "TS Processing Thread Terminated");
    isRunning = false;
}

void MuxMPEWorker::configureFromFile(std::string conf_file)
{

    if (stringEndsWith(conf_file, ".json"))
    {
        read_json(conf_file, pt);
    }
    else
    {
        read_info(conf_file, pt);
    }
    configureAll();
}

void MuxMPEWorker::configureFromJSON(std::string json)
{
    config = json;
    std::istringstream jsons(json);
    read_json(jsons, pt);
    configureAll();
}

std::string MuxMPEWorker::getConfig()
{
    return config;
}

void MuxMPEWorker::configureAll()
{
    inputs.clear();
    // unicastinputs.clear();
    //  Setup Inputs
    set<string> all_input_names;

    etiLog.log(info, "Setting New Config");
    boost::property_tree::write_json(std::cout, pt, false);

    if (pt.get_child("general").get<string>("tsduck_debug") == "true")
    {
        debuglevel = ts::Severity::Debug;
    }
    else
    {
        debuglevel = ts::Severity::Info;
    }

    if (pt.count("unicasttomcast"))
    {
        auto unicasttomcast_node = pt.get_child("unicasttomcast");

        tap_interface = unicasttomcast_node.get<std::string>("tap_interface");
        tap_ip = unicasttomcast_node.get<std::string>("tap_ip");
        tap_subnet_mask = unicasttomcast_node.get<std::string>("tap_subnet_mask");

        for (const auto &input_node : unicasttomcast_node.get_child("inputs"))
        {
            auto uni2mcast = make_shared<unicast_to_mcast_t>();

            uni2mcast->listenport = input_node.second.get<int>("listenport");
            uni2mcast->mcast_address = input_node.second.get<std::string>("mcast_address");
            uni2mcast->mcast_port = input_node.second.get<int>("mcast_port");
            unicastinputs.push_back(uni2mcast);
        }
    }
    else
    {
        std::cerr << "No unicasttomcast node found in JSON file" << std::endl;
    }

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

        // Build input
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
    dest = new ts_destination_t();
    dest->bitrate = pt_output.get<unsigned int>("bitrate");
    dest->output = pt_output.get<string>("output");
    dest->payload_pid = pt_output.get<unsigned int>("payload_pid");
    dest->pmt_pid = pt_output.get<unsigned int>("pmt_pid");
    dest->ts_id = pt_output.get<unsigned int>("ts_id");
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

    haveConfig = true;
}

bool MuxMPEWorker::status()
{
    if (tsproc != nullptr && tsproc->isStarted())
    {
        return true;
    }
    else
    {
        return false;
    }
}