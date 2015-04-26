#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>
#include <tuple>
#include <cstdint>
#include <cstdlib>

#include <boost/fusion/include/io.hpp>
#include <boost/fusion/include/as_vector.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>

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

int ntp_main(int sockfd) {
    NTP_msg msg = {0};
    int cnt;
    msg.header = mkHeader();
    // for(int i(0); i < sizeof(NTP_msg); ++i)
    // 	std::cout << ((int)(reinterpret_cast<const unsigned char *>(&msg))[i]) << std::endl;

    cnt = write(sockfd, &msg, sizeof(msg));
    std::cout << "send " << cnt << " byte(s)!" << std::endl;
    NTP_msg reply;
    read(sockfd, &reply, sizeof(reply));

    std::cout << boost::fusion::as_vector(readHeader(reply.header)) << std::endl;
    // std::cout << reply.header << std::endl;
    // for(int i(0); i < sizeof(NTP_msg); ++i)
    // 	std::cout << ((int)(reinterpret_cast<const unsigned char *>(&reply))[i]) << std::endl;
    // char buf;
    // while(read(sockfd, &buf, 1)) std::cout << (int) buf << std::endl;
}

int main(int, char**) {
    struct sockaddr_in addr = {0};
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0) std::exit(-1);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("10.8.8.12");
    addr.sin_port = htons(123);

    if (addr.sin_addr.s_addr == -1) std::cerr << "wrong!" << std::endl;
    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
	std::exit(-2);

    ntp_main(sockfd);

    close(sockfd);

    return 0;
}
