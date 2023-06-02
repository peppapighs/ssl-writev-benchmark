/*
 * This file contains the configuration parameters for the application.
 */

// Name of this benchmark to be printed.
#define STAT_NAME "SSL_writev()"
// Port to listen on.
#define PORT 4433
// Random seed for the PRNG.
#define RANDOM_SEED 42
// Number of packets to send.
#define NUM_PACKETS (1 << 18)
// Number of packets to send in each writev call. Set to 1 to use SSL_write()
// instead.
#define NUM_IOV (1 << 1)
// Number of bytes to send in each packet.
#define MIN_PACKET_SIZE 200
#define MAX_PACKET_SIZE 250
// Enable cache flushing.
#define FLUSH_CACHE 0