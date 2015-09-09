#include <inttypes.h> // PRIu8, PRIu32
#include <netdb.h> // getaddrinfo, struct addrinfo
#include <netinet/in.h> // IPPROTO_UDP
#include <stdlib.h> // exit
#include <stdint.h> // uint8_t, uint32_t
#include <stdio.h> // printf, fprintf, perror
#include <string.h> // memcpy
#include <sys/socket.h> // socket, setsockopt, SOCK_DGRAM, sendto, recvfrom
#include <sys/time.h> // struct timeval
#include <time.h> // localtime, strftime, struct tm
#include <unistd.h> // close

#include "msntpc.h"


int main(int argc, char *argv[]) {
	char * server_addr = "0.pool.ntp.org";
	if (argc == 2) {
		server_addr = argv[1];
	}

	uint8_t sntp_request[SNTP_DATA_LEN] = {0};
	uint8_t sntp_response[SNTP_DATA_LEN] = {0};

	sntp_request[0] = SNTP_LI_ALARM_CONDITION | SNTP_VERSION | SNTP_MODE_CLIENT;

	printf("Requesting current time from: %s\n", server_addr);
	ssize_t size = send_request(server_addr, sntp_request, sntp_response);
	if (size != SNTP_DATA_LEN) {
		fprintf(stderr, "Invalid response length: %zu, should be %d\n", size, SNTP_DATA_LEN);
		exit(2);
	}

	sntp_packet * packet = parse_response(sntp_response);
	print_response(packet);
}


ssize_t send_request(char * server_addr, uint8_t * request, uint8_t * response) {
	struct addrinfo * addr;
	if (getaddrinfo(server_addr, "123", 0, &addr)) {
		perror("send_request/getaddrinfo");
	}

	int sock = socket(addr->ai_family, SOCK_DGRAM, IPPROTO_UDP);
	if (sock < 0) {
		perror("send_request/socket");
	}

	struct timeval timeout = { .tv_sec = SNTP_RECV_TIMEOUT };
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

	if (sendto(sock, request, SNTP_DATA_LEN, 0, addr->ai_addr, addr->ai_addrlen) < 0) {
		perror("send_request/sendto");
	}

	ssize_t size = 0;
	if ((size = recvfrom(sock, response, SNTP_DATA_LEN, 0, addr->ai_addr, &addr->ai_addrlen)) < 0) {
		perror("send_request/recvfrom");
	}

	close(sock);
	return size;
}

sntp_packet * parse_response(uint8_t * response) {
	sntp_packet * packet = malloc(sizeof(*packet));

	uint8_t li_vn_mode;
	memcpy(&li_vn_mode, response, 1);

	packet->li = SNTP_PACKET_LEAP_INDICATOR(response);
	packet->ver = SNTP_PACKET_VERSION(response);
	packet->mode = SNTP_PACKET_MODE(response);

	uint8_t stratum;
	memcpy(&stratum, (response + SNTP_STRATUM_OFFSET), sizeof(stratum));
	packet->stratum = stratum;

	int8_t precision;
	memcpy(&precision, (response + SNTP_PRECISION_OFFSET), sizeof(precision));
	packet->precision = precision;

	uint32_t timestamp;
	memcpy(&timestamp, (response + SNTP_TRANS_TS_OFFSET), sizeof(timestamp));
	packet->trans_ts = ntohl(timestamp) - DIFF_UNIX_SNTP;

	return packet;
}

void print_response(sntp_packet * response) {
	printf("Leap indicator: %s\n",           li_strings[response->li]);
	printf("Version number: %" PRIu8 "\n",              response->ver);
	printf(" Response mode: %s\n",         mode_strings[response->mode]);
	printf("       Stratum: %" PRIu8 "\n",              response->stratum);
	printf("     Precision: %" PRId8 "\n",              response->precision);
	printf("    UNIX epoch: %" PRIu32 "\n",             response->trans_ts);

	struct timeval timestamp_tv = { .tv_sec = response->trans_ts };
	struct tm    * timestamp_tm = localtime(&timestamp_tv.tv_sec);
	char tmbuf[26];
	strftime(tmbuf, sizeof(tmbuf), "%FT%T%z", timestamp_tm);
	printf("      ISO-8601: %s\n", tmbuf);
}
