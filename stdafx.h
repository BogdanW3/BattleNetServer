#ifndef HEADER_49B89A38D3B43576
#define HEADER_49B89A38D3B43576

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <fstream>
#include <thread>
#include <synchapi.h>
#include <chrono>
#include <map>
#include <tuple>
#include <openssl/sha.h>
#include <boost/multiprecision/cpp_int.hpp>
#include <ctype.h>
#include <cmath>
#include <mutex>

namespace mp = boost::multiprecision;
using namespace mp::literals;
std::map<std::string, std::tuple<std::string, std::string, std::string>> users;
std::map<std::string, std::tuple<std::string, std::string, std::string, unsigned int>> games; //pass, gamestat, host IP and port(it's in reverse order)
std::ofstream packets("E:\\Packets", std::ios::binary);
std::mutex mtx;
unsigned int recvbuflen = 1024;
thread_local int nls=2;
thread_local int verbyte=0;
thread_local int demo=0;
mp::uint256_t g = 0x2F_cppui256;
mp::uint256_t b = 0x322488600753_cppui256;
mp::uint256_t N = 0xF8FF1A8B619918032186B68CA092B5557E976C78C73212D91216F6658523C787_cppui256;
unsigned char Nch[] = {0x87, 0xC7, 0x23, 0x85, 0x65, 0xF6, 0x16, 0x12, 0xD9, 0x12, 0x32, 0xC7, 0x78, 0x6C, 0x97, 0x7E, 0x55, 0xB5, 0x92, 0xA0, 0x8C, 0xB6, 0x86, 0x21, 0x03, 0x18, 0x99, 0x61, 0x8B, 0x1A, 0xFF, 0xF8};
//Above are N and Nch for version 2, the bottom ones are for version 1
//mp::uint256_t N1 = 0x87C7238565F61612D91232C7786C977E55B592A08CB68621031899618B1AFFF8_cppui256;
//unsigned char Nch1[] = {0xF8, 0xFF, 0x1A, 0x8B, 0x61, 0x99, 0x18, 0x03, 0x21, 0x86, 0xB6, 0x8C, 0xA0, 0x92, 0xB5, 0x55, 0x7E, 0x97, 0x6C, 0x78, 0xC7, 0x32, 0x12, 0xD9, 0x12, 0x16, 0xF6, 0x65, 0x85, 0x23, 0xC7, 0x87};

#include "uint_math.h"
#include "Packets.h"
#include "MainGameLoop.h"
#include "Timer.h"
#include "Telnet.h"
#endif // header guard 
