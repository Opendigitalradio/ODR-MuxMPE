#ifndef MUX_MPE_WORKER_H
#define MUX_MPE_WORKER_H

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <vector>
#include "../lib/Log.h"
#include "utils.h"
#include <iterator>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <functional>
#include <algorithm>
#include <tsduck.h>
#include <iostream>
#include <thread>
#include "MulticastTap.h"

using namespace std;
using boost::property_tree::ptree;

class MuxMPEWorker
{
public:
  MuxMPEWorker();
  ~MuxMPEWorker();
  bool startup();
  bool stop();
  void configureFromFile(std::string config_file);
  void configureFromJSON(std::string config_file);
  bool status();
  void startUpWithConf();
  int createTapInterface(std::string tap_name, std::string tap_ip);
  std::string getConfig();

private:
  void configureAll();
  void BringUpMux();
  std::thread *ts_thread;
  bool haveConfig = false;
  bool isRunning = false;
  int debuglevel = -1;
  ptree pt;
  std::vector<std::shared_ptr<udp_source_t>> inputs;
  std::vector<std::shared_ptr<unicast_to_mcast_t>> unicastinputs;
  ts_destination_t *dest;
  ts::TSProcessor *tsproc;
  std::string config;
  std::string tap_interface;
  std::string tap_ip;
  std::string tap_subnet_mask;
  bool use_tap = false;
  bool tap_interface_exists(const std::string& tap_interface_name);
};

#endif // MUX_MPE_WORKER_H