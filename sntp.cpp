#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>
#include <tuple>
#include <string>
#include <sstream>
#include <cstdint>
#include <cstdlib>
#include <ctime>

#include <boost/fusion/include/io.hpp>
#include <boost/fusion/include/as_vector.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>

// rfc2030 を読んで実装。

struct TimeStamp {
    uint32_t time[2];
};

struct NTP_msg {
    uint32_t header;
    uint32_t rootDelay;
    uint32_t rootDisp;
    uint32_t refIdent;
    TimeStamp refTS;
    TimeStamp origTS;
    TimeStamp recTS;
    TimeStamp transTS;
};

uint32_t mkHeader() {
    return htonl(
      (0 << 30) | // LI
      (4 << 27) | // VN
      (3 << 24) | // MODE
      0); // other
}

std::tuple<int, int, int, int, int, int> readHeader(uint32_t header) {
    int li, vn, mode, stratum, poll;
    signed char precision;
    header = htonl(header);
    li = ((3 << 30) & header) >> 30;
    vn = ((7 << 27) & header) >> 27;
    mode = ((7 << 24) & header) >> 24;
    stratum = ((255 << 16) & header) >> 16;
    poll = ((255 << 8) & header) >> 8;
    precision = 255 & header;
    return std::make_tuple(li, vn, mode, stratum, poll, precision);
}

double conv16_16(uint32_t val) {
    double v = val;
    v /= (1 << 16);
    return v;
}

long double convNTPTimeStamp(TimeStamp ts) {
    long double v;
    v = ts.time[0];
    // v += (ts.time[1] * 1.0) / (2LL << 32);
    return v;
}

std::string refIdent2str(uint32_t header, uint32_t ident) {
    std::string str;
    if(std::get<3>(readHeader(header)) < 2) { // stratum == 0 or 1 then some string
      ident = htonl(ident);
      str = "";
      char id[4];
      id[0] = ((255 << 24) & ident) >> 24;
      id[1] = ((255 << 16) & ident) >> 16;
      id[2] = ((255 <<  8) & ident) >>  8;
      id[3] = ((255 <<  0) & ident) >>  0;
      str += id[0];
      str += id[1];
      str += id[2];
      str += id[3];
    } else { // other : ipv4 addr <- sntp v3
      // ident = htonl(ident);
      // int addr[4];
      // addr[0] = ((255 << 24) & ident) >> 24;
      // addr[1] = ((255 << 16) & ident) >> 16;
      // addr[2] = ((255 <<  8) & ident) >>  8;
      // addr[3] = ((255 <<  0) & ident) >>  0;
      // std::stringstream ss;
      // ss << addr[0] << '.' << addr[1] << '.' << addr[2] << '.' << addr[3];
      // str = ss.str();
      str = "(snip)"; // in sntp v4:  this is the low order 32 bits of the latest transmit timestamp of the reference source.
      // あんまり出してもおもしろいものでもないので飛ばす。
    }
    return str;
}

int ntp_main(int sockfd) {
    struct timeval t;
    gettimeofday(&t, nullptr);
    NTP_msg msg = {0};
    int cnt;
    msg.header = mkHeader();
    msg.transTS.time[0] = htonl(t.tv_sec + 2208988800);
    msg.transTS.time[1] = 0; // htonl(t.tv_usec); // <- これはマイクロセカンド単位だ。
    // for(int i(0); i < sizeof(NTP_msg); ++i)
    // 	std::cout << ((int)(reinterpret_cast<const unsigned char *>(&msg))[i]) << std::endl;

    cnt = write(sockfd, &msg, sizeof(msg));
    std::cout << "send " << cnt << " byte(s)!" << std::endl;
    NTP_msg reply;
    read(sockfd, &reply, sizeof(reply));

    std::cout << "header: " << boost::fusion::as_vector(readHeader(reply.header)) << std::endl;
    std::cout << "rootDelay: " << conv16_16(reply.rootDelay) << std::endl;
    // std::cout << "rootDelay: " << (reply.rootDelay) << std::endl;
    std::cout << "rootDisp: " << conv16_16(reply.rootDisp) << std::endl;
    // std::cout << "rootDisp: " << (reply.rootDisp) << std::endl;
    std::cout << "refIdent: " << refIdent2str(reply.header, reply.refIdent) << std::endl;
    std::cout << "refTS: " << convNTPTimeStamp(reply.refTS) << std::endl;
    std::cout << "origTS: " << convNTPTimeStamp(reply.origTS) << std::endl;
    std::cout << "recTS: " << convNTPTimeStamp(reply.recTS) << std::endl;

    std::cout << "  refTS:\t" << htonl(reply.refTS.time[0]) << ", " << htonl(reply.refTS.time[1]) << std::endl;
    std::cout << " origTS:\t" << htonl(reply.origTS.time[0]) << ", " << htonl(reply.origTS.time[1]) << std::endl;
    std::cout << "  recTS:\t" << htonl(reply.recTS.time[0]) << ", " << htonl(reply.recTS.time[1]) << std::endl;
    std::cout << "transTS:\t" << htonl(reply.transTS.time[0]) << ", " << htonl(reply.transTS.time[1]) << std::endl;
    // std::cout << reply.header << std::endl;
    // for(int i(0); i < sizeof(NTP_msg); ++i)
    // 	std::cout << ((int)(reinterpret_cast<const unsigned char *>(&reply))[i]) << std::endl;
    // char buf;
    // while(read(sockfd, &buf, 1)) std::cout << (int) buf << std::endl;
}

int main(int argc, char** argv) {
    if(argc == 1) {
      std::cerr << argv[0] << "NTP_SERVER_ADDR" << std::endl;
      std::cerr << "example: " << std::endl;
      std::cerr << argv[0] << " 133.243.238.244" << std::endl;
      exit(-1);
    }
    std::cout << "Now: " << (std::time(nullptr) + 2208988800) << std::endl;
    struct sockaddr_in addr = {0};
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0) std::exit(-1);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(123);

    if (addr.sin_addr.s_addr == -1) std::cerr << "wrong!" << std::endl;
    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
      std::exit(-2);

    ntp_main(sockfd);

    close(sockfd);

    return 0;
}
