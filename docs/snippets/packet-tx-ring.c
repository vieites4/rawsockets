// Example asynchronous packet socket reading with PACKET_TX_RING
// From http://codemonkeytips.blogspot.com/2011/07/asynchronous-packet-socket-writing-with.html

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
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <linux/if.h>
#include <linux/if_packet.h>

/// The number of frames in the ring
//  This number is not set in stone. Nor are block_size, block_nr or frame_size
#define CONF_RING_FRAMES        128

#define CONF_DEVICE		"eth0"

/// Offset of data from start of frame
#define PKT_OFFSET      (TPACKET_ALIGN(sizeof(struct tpacket_hdr)) + \
                         TPACKET_ALIGN(sizeof(struct sockaddr_ll)))

/// (unimportant) macro for loud failure
#define RETURN_ERROR(lvl, msg) \
  do {                    \
    fprintf(stderr, msg); \
    return lvl;            \
  } while(0);

static struct sockaddr_ll txring_daddr;

/// create a linklayer destination address
//  @param ringdev is a link layer device name, such as "eth0"
static int
init_ring_daddr(int fd, const char *ringdev)
{
  struct ifreq ifreq;

  // get device index
  strcpy(ifreq.ifr_name, ringdev);
  if (ioctl(fd, SIOCGIFINDEX, &ifreq)) {
    perror("ioctl");
    return -1;
  }

  txring_daddr.sll_family    = AF_PACKET;
  txring_daddr.sll_protocol  = htons(ETH_P_IP);
  txring_daddr.sll_ifindex   = ifreq.ifr_ifindex;

  // set the linklayer destination address
  // NOTE: this should be a real address, not ff.ff....
  txring_daddr.sll_halen     = ETH_ALEN;
  memset(&txring_daddr.sll_addr, 0xff, ETH_ALEN);
  return 0;
}

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

  if (init_ring_daddr(fd, CONF_DEVICE))
   return NULL;

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

/// transmit a packet using packet ring
//  NOTE: for high rate processing try to batch system calls, 
//        by writing multiple packets to the ring before calling send()
//
//  @param pkt is a packet from the network layer up (e.g., IP)
//  @return 0 on success, -1 on failure
static int
process_tx(int fd, char *ring, const char *pkt, int pktlen)
{
  static int ring_offset = 0;

  struct tpacket_hdr *header;
  struct pollfd pollset;
  char *off;
  int ret;

  // fetch a frame
  // like in the PACKET_RX_RING case, we define frames to be a page long,
  // including their header. This explains the use of getpagesize().
  header = (void *) ring + (ring_offset * getpagesize());
  assert((((unsigned long) header) & (getpagesize() - 1)) == 0);
  while (header->tp_status != TP_STATUS_AVAILABLE) {

    // if none available: wait on more data
    pollset.fd = fd;
    pollset.events = POLLOUT;
    pollset.revents = 0;
    ret = poll(&pollset, 1, 1000 /* don't hang */);
    if (ret < 0) {
      if (errno != EINTR) {
        perror("poll");
        return -1;
      }
      return 0;
    }
  }

  // fill data
  off = ((void *) header) + (TPACKET_HDRLEN - sizeof(struct sockaddr_ll));
  memcpy(off, pkt, pktlen);

  // fill header
  header->tp_len = pktlen;
  header->tp_status = TP_STATUS_SEND_REQUEST;

  // increase consumer ring pointer
  ring_offset = (ring_offset + 1) & (CONF_RING_FRAMES - 1);

  // notify kernel
  if (sendto(fd, NULL, 0, 0, (void *) &txring_daddr, sizeof(txring_daddr)) < 0) {
    perror("sendto");
    return -1;
  }

  return 0;
}

/// Example application that opens a packet socket with rx_ring
int
main(int argc, char **argv)
{
  char *ring, pkt[100];
  int fd;

  fd = init_packetsock(&ring, PACKET_TX_RING);
  if (fd < 0)
    return 1;

  // TODO: make correct IP packet out of pkt
  process_tx(fd, ring, pkt, 100);

  if (exit_packetsock(fd, ring))
    return 1;

  printf("OK\n");
  return 0;
}


