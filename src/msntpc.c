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

/*  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |LI | VN  |Mode |    Stratum    |     Poll      |   Precision   |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                          Root  Delay                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                       Root  Dispersion                        |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                     Reference Identifier                      |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                               |
 * |                    Reference Timestamp (64)                   |
 * |                                                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                               |
 * |                    Originate Timestamp (64)                   |
 * |                                                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                               |
 * |                     Receive Timestamp (64)                    |
 * |                                                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                               |
 * |                     Transmit Timestamp (64)                   |
 * |                                                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                 Key Identifier (optional) (32)                |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                               |
 * |                                                               |
 * |                 Message Digest (optional) (128)               |
 * |                                                               |
 * |                                                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define SNTP_DATA_LEN 48

#define SNTP_LI_NO_WARN         (0x00 << 6)
#define SNTP_LI_LAST_SIXTYONE   (0x01 << 6)
#define SNTP_LI_LAST_FIFTYNINE  (0x02 << 6)
#define SNTP_LI_ALARM_CONDITION (0x03 << 6)

#define SNTP_VERSION (4 << 3) /* Version 4 */

#define SNTP_MODE_CLIENT    0x03
#define SNTP_MODE_SERVER    0x04
#define SNTP_MODE_BROADCAST 0x05
#define SNTP_MODE_MASK      0x07


#define SNTP_STRATUM_PRIMARY       0x01
#define SNTP_STRATUM_SECONDARY_MIN 0x02
#define SNTP_STRATUM_SECONDARY_MAX 0x0F


#define SNTP_RECV_TIMEOUT           3000

#define SNTP_TRANS_TS_OFFSET 32 /* 4 fields * 8 rows */
#define SNTP_STRATUM_OFFSET  1

// Seconds between 1900 (SNTP epoch) and 1970 (UNIX epoch)
#define DIFF_UNIX_SNTP 2208988800


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
		perror("send_request");
	}

	int sock = socket(addr->ai_family, SOCK_DGRAM, IPPROTO_UDP);
	if (sock < 0) {
		printf("Socket creation failed.\n");
	}

	int timeout = SNTP_RECV_TIMEOUT;
	setsockopt( sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

	if (sendto(sock, request, SNTP_DATA_LEN, 0, addr->ai_addr, addr->ai_addrlen) < 0) {
		perror("send_request");
	}

	ssize_t size = 0;
	if ((size = recvfrom(sock, response, SNTP_DATA_LEN, 0, addr->ai_addr, &addr->ai_addrlen)) < 0) {
		perror("recvfrom");
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
