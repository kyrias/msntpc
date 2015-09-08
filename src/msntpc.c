#include <stdlib.h> // exit
#include <stdio.h> // printf
#include <string.h> // memcpy
#include <stdint.h> // uint8_t, uint32_t
#include <inttypes.h> // PRIu8, PRIu32
#include <unistd.h> // close
#include <sys/socket.h>
#include <arpa/inet.h> // sockaddr_in, htons, inet_aton
#include <netdb.h> // struct addrinfo

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

	print_response(sntp_response);
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


void print_response(uint8_t * response) {
	printf("Response mode: %" PRIu8 "\n", (response[0] & SNTP_MODE_MASK));

	uint32_t timestamp;
	memcpy(&timestamp, (response + SNTP_TRANS_TS_OFFSET), sizeof(timestamp));
	printf("UNIX epoch: %" PRIu32 "\n", (uint32_t)(ntohl(timestamp) - DIFF_UNIX_SNTP));


	uint8_t stratum;
	memcpy(&stratum, (response + SNTP_STRATUM_OFFSET), sizeof(stratum));
	printf("Stratum: %" PRIu8 "\n", stratum);
}
