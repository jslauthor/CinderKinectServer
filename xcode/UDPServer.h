//
//  UDPServer.h
//  CinderKinectServer
//
//  Created by Leonard Souza on 3/15/11.
//  Copyright 2011 effectiveui. All rights reserved.
//

#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::udp;
using namespace std;

#define MAX_PACKET_SIZE 1500

class UDPServer
{
    public:
        UDPServer(boost::asio::io_service& io_service) : socket_(io_service)
        {
            socket_.open(udp::v4());
        }
        
        void sendMessage(int first, int second, unsigned char *data, int len)
        {
            printf("sending message\n");
            try
            {
                unsigned char *message = new unsigned char[MAX_PACKET_SIZE];
                
                int m_len = 1 + 1;
                memcpy(message, &first, 1);
                memcpy(message + 1, &second, 1);
                memcpy(message + m_len, data, len);
                
                socket_.send_to(boost::asio::buffer(message, m_len + len), udp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 3131));
            }
            catch (std::exception& e)
            {
                cout << e.what() << endl;
            }
        }
    private:
        udp::socket socket_;
        udp::endpoint remote_endpoint_;
};