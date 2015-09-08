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


ssize_t send_request(char *, uint8_t *, uint8_t *);
void print_response(uint8_t *);
