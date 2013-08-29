// Example asynchronous packet socket reading with PACKET_RX_RING
// From http://codemonkeytips.blogspot.com/2011/07/asynchronous-packet-socket-reading-with.html

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>

#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <linux/if_packet.h>

/// The number of frames in the ring
//  This number is not set in stone. Nor are block_size, block_nr or frame_size
#define CONF_RING_FRAMES          128

/// Offset of data from start of frame
#define PKT_OFFSET      (TPACKET_ALIGN(sizeof(struct tpacket_hdr)) + \
                         TPACKET_ALIGN(sizeof(struct sockaddr_ll)))

/// (unimportant) macro for loud failure
#define RETURN_ERROR(lvl, msg) \
  do {                    \
    fprintf(stderr, msg); \
    return lvl;            \
  } while(0);

static int rxring_offset;

/// Initialize a packet socket ring buffer
//  @param ringtype is one of PACKET_RX_RING or PACKET_TX_RING
static char *
init_packetsock_ring(int fd, int ringtype)
{
  struct tpacket_req tp;
  char *ring;

  // tell kernel to export data through mmap()ped ring
  tp.tp_block_size = CONF_RING_FRAMES * getpagesize();
  tp.tp_block_nr = 1;
  tp.tp_frame_size = getpagesize();
  tp.tp_frame_nr = CONF_RING_FRAMES;
  if (setsockopt(fd, SOL_PACKET, ringtype, (void*) &tp, sizeof(tp)))
    RETURN_ERROR(NULL, "setsockopt() ring\n");

#ifdef TPACKET_V2
  val = TPACKET_V1;
  setsockopt(fd, SOL_PACKET, PACKET_HDRLEN, &val, sizeof(val));
#endif

  // open ring
  ring = mmap(0, tp.tp_block_size * tp.tp_block_nr,
               PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (!ring)
    RETURN_ERROR(NULL, "mmap()\n");

  return ring;
}

/// Create a packet socket. If param ring is not NULL, the buffer is mapped
//  @param ring will, if set, point to the mapped ring on return
//  @return the socket fd
static int
init_packetsock(char **ring, int ringtype)
{
  int fd;

  // open packet socket
  fd = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP));
  if (fd < 0)
    RETURN_ERROR(-1, "Root priliveges are required\nsocket() rx. \n");

  if (ring) {
    *ring = init_packetsock_ring(fd, ringtype);

    if (!*ring) {
      close(fd);
      return -1;
    }
  }

  return fd;
}

static int
exit_packetsock(int fd, char *ring)
{
  if (munmap(ring, CONF_RING_FRAMES * getpagesize())) {
    perror("munmap");
    return 1;
  }

  if (close(fd)) {
    perror("close");
    return 1;
  }

  return 0;
}

/// Blocking read, returns a single packet (from packet ring)
static void *
process_rx(const int fd, char *rx_ring)
{
  struct tpacket_hdr *header;
  struct pollfd pollset;
  int ret;

  // fetch a frame
  header = (void *) rx_ring + (rxring_offset * getpagesize());
  assert((((unsigned long) header) & (getpagesize() - 1)) == 0);

  // TP_STATUS_USER means that the process owns the packet.
  // When a slot does not have this flag set, the frame is not
  // ready for consumption.
  while (!(header->tp_status & TP_STATUS_USER)) {

    // if none available: wait on more data
    pollset.fd = fd;
    pollset.events = POLLIN;
    pollset.revents = 0;
    ret = poll(&pollset, 1, -1 /* negative means infinite */);
    if (ret < 0) {
      if (errno != EINTR)
        RETURN_ERROR(NULL, "poll()\n");
      return NULL;
    }
  }

  // check data
  if (header->tp_status & TP_STATUS_COPY)
    RETURN_ERROR(NULL, "skipped: incomplete packed\n");
  if (header->tp_status & TP_STATUS_LOSING)
    fprintf(stderr, "dropped packets detected\n");

  // return encapsulated packet
  return ((void *) header) + PKT_OFFSET;
}

// Release the slot back to the kernel
static void
process_rx_release(char *rx_ring)
{
  struct tpacket_hdr *header;

  // clear status to grant to kernel
  header = (void *) rx_ring + (rxring_offset * getpagesize());
  header->tp_status = 0;

  // update consumer pointer
  rxring_offset = (rxring_offset + 1) & (CONF_RING_FRAMES - 1);
}

/// Example application that opens a packet socket with rx_ring
int
main(int argc, char **argv)
{
  char *ring, *pkt;
  int fd;

  fd = init_packetsock(&ring, PACKET_RX_RING);
  if (fd < 0)
    return 1;

  pkt = process_rx(fd, ring);
  process_rx_release(ring);

  if (exit_packetsock(fd, ring))
    return 1;

  printf("OK\n");
  return 0;
}


