/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "net/InetAddress.h"

#include <cassert>
#include <iostream>

#include "base/Logging.h"

using namespace web_server::net;

int main() {
    InetAddress addr0(1234);
    std::cout << addr0.to_IP() << std::endl;
    std::cout << addr0.to_IP_port() << std::endl;
    InetAddress addr1("1.2.3.4", 8888);
    std::cout << addr1.to_IP() << std::endl;
    std::cout << addr1.to_IP_port() << std::endl;
    InetAddress addr2("255.254.253.252", 65535);
    std::cout << addr2.to_IP() << std::endl;
    std::cout << addr2.to_IP_port() << std::endl;
}