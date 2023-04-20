#ifndef MULTICAST_TAP_H
#define MULTICAST_TAP_H

#include <arpa/inet.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <linux/if_packet.h>
#include <linux/if_tun.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

class MulticastTap {
public:
    MulticastTap(const std::string& tap_name, const std::string& multicast_ip, int recv_port, int send_port, int buffer_size);
    
    void udp_to_multicast();
    void run();

private:
    std::string tap_name_;
    std::string multicast_ip_;
    int recv_port_;
    int send_port_;
    int buffer_size_;
    int tap_fd_;
};

#endif // MULTICAST_TAP_H
