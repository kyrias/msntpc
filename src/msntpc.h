#include <sys/types.h>
#include <stdint.h>

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

typedef enum {
	NO_WARNING,
	LAST_SIXTYONE,
	LAST_FIFTYNINE,
	ALARM_CONDITION,
} leap_indicator;

const static char * li_strings[] = {
	[NO_WARNING] = "No warning",
	[LAST_SIXTYONE] = "Last minute has 61 seconds",
	[LAST_FIFTYNINE] = "Last minute has 59 seconds",
	[ALARM_CONDITION] = "Alarm condition (clock not synchronized)",
};

const static char * mode_strings[] = {
	[0] = "Reserved",
	[1] = "Symmetric active",
	[2] = "Symmetric passive",
	[3] = "Client",
	[4] = "Server",
	[5] = "Broadcast",
	[6] = "Reserved for NTP control message",
	[7] = "Reserved for private use",
};

typedef struct sntp_packet {
	leap_indicator li;
	uint8_t        ver;
	uint8_t        mode;
	uint8_t        stratum;
	int8_t         precision;
	uint32_t       trans_ts;
} sntp_packet;

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

// Macros for getting out specific values from packet fields
#define SNTP_PACKET_LEAP_INDICATOR(PACKET) (PACKET[0] >> 6)
#define SNTP_PACKET_VERSION(PACKET) ((PACKET[0] & 0x38) >> 3)
#define SNTP_PACKET_MODE(PACKET) (PACKET[0] & 0x07)

#define SNTP_RECV_TIMEOUT           3000

#define SNTP_TRANS_TS_OFFSET  32 /* 4 fields * 8 rows */
#define SNTP_STRATUM_OFFSET   1
#define SNTP_PRECISION_OFFSET 3

// Seconds between 1900 (SNTP epoch) and 1970 (UNIX epoch)
#define DIFF_UNIX_SNTP 2208988800


ssize_t send_request(char *, uint8_t *, uint8_t *);
sntp_packet * parse_response(uint8_t * response);
void print_response(sntp_packet *);
