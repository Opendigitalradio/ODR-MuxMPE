#include "MulticastTap.h"

    MulticastTap::MulticastTap(const std::string& tap_name, const std::string& multicast_ip, int recv_port, int send_port, int buffer_size)
    : tap_name_(tap_name)
    , multicast_ip_(multicast_ip)
    , recv_port_(recv_port)
    , send_port_(send_port)
    , buffer_size_(buffer_size)
{}

    
    void MulticastTap::udp_to_multicast()
    {

        // Create input socket
        int input_socket = socket(AF_INET, SOCK_DGRAM, 0);
        if (input_socket < 0)
        {
            perror("input socket creation failed");
            exit(EXIT_FAILURE);
        }

        int reuse = 1;
        setsockopt(input_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

        // Bind input socket to input port
        struct sockaddr_in input_address;
        memset(&input_address, 0, sizeof(input_address));
        input_address.sin_family = AF_INET;
        input_address.sin_addr.s_addr = INADDR_ANY;
        input_address.sin_port = htons(recv_port_);
        if (bind(input_socket, (struct sockaddr *)&input_address, sizeof(input_address)) < 0)
        {
            printf("input bind failed: %d", recv_port_);
            exit(EXIT_FAILURE);
        }

        // Set multicast output address
        struct sockaddr_in output_address;
        memset(&output_address, 0, sizeof(output_address));
        output_address.sin_family = AF_INET;
        output_address.sin_addr.s_addr = inet_addr(multicast_ip_.c_str());
        output_address.sin_port = htons(send_port_);

        // Create tap socket
        int tap_socket = socket(AF_INET, SOCK_DGRAM, 0);
        if (tap_socket < 0)
        {
            perror("tap socket creation failed");
            exit(EXIT_FAILURE);
        }

        //printf("name: %s\n", tap_name_.c_str());
        if (setsockopt(tap_socket, SOL_SOCKET, SO_BINDTODEVICE, tap_name_.c_str(), strlen(tap_name_.c_str())) < 0)
        {
            perror("tap setsockopt failed");
            exit(EXIT_FAILURE);
        }

        // Receive and multicast packets
        char buffer[buffer_size_];
        int n;
        while (true)
        {
            n = recv(input_socket, buffer, sizeof(buffer), 0);
            if (n < 0)
            {
                perror("input recv failed");
                exit(EXIT_FAILURE);
            }
            if (sendto(tap_socket, buffer, n, 0, (struct sockaddr *)&output_address, sizeof(output_address)) < 0)
            {
                perror("tap sendto failed");
                exit(EXIT_FAILURE);
            }
        }
    }

    void MulticastTap::run()
    {
        udp_to_multicast();
    }