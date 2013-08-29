/*
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 */

#include <errno.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>

#define DEFAULT_IF	"eth0"
#define BUF_SIZ		2048

#define USE_WRITE 1
#define GN_ETHTYPE 0x0707

/*!< Ethernet broadcast address. */
const unsigned char ETH_ADDR_BROADCAST[ETH_ALEN]
                                    = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

/*!< Ethernet NULL address. */
const unsigned char ETH_ADDR_NULL[ETH_ALEN]
                                    = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
/*!< Ethernet FAKE address. */
const unsigned char ETH_ADDR_FAKE[ETH_ALEN]
                                    = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55 };
/*!< Ethernet SISCOGA-1-APU address. */
const unsigned char ETH_ADDR_S1A[ETH_ALEN]
                                    = { 0x00, 0x04, 0x5F, 0x03, 0xED, 0xCA };

int print_hex_data(const char *buffer, const int len);

typedef struct llc_header
{
	uint16_t ssap;
	uint16_t dsap;
}
llc_header_t;

#define LEN__ETH_HEADER 	sizeof(struct ethhdr)
#define LEN__LLC_HEADER_T 	sizeof(llc_header_t)

#define LEN__HEADERS 		( LEN__ETH_HEADER + LEN__LLC_HEADER_T )
#define LEN__DATA 			1024
#define LEN__FRAME			LEN__HEADERS + LEN__DATA

int main(int argc, char *argv[])
{

	int sockfd;
	uint16_t lsap = htons(ETH_P_IP);
	struct ifreq if_idx;
	struct ifreq if_mac;

	char sendbuf[BUF_SIZ];
	struct ethhdr *eh = (struct ethhdr *) sendbuf;
	llc_header_t *llc_header = (llc_header_t *) ( sendbuf + LEN__ETH_HEADER );
	char *data = (char *) sendbuf + LEN__HEADERS;

	char ifName[IFNAMSIZ];

	/* Get interface name */
	if (argc > 1) strcpy(ifName, argv[1]);
	else strcpy(ifName, DEFAULT_IF);

	/* Open RAW socket to send on */
	//IPPROTO_RAW changed by htons(GN_ETHTYPE)
	if ( (sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0 )
	{
	    perror("Could not create socket...");
	    exit(-1);
	}

	/* Get the index of the interface to send on */
	memset(&if_idx, 0, sizeof(struct ifreq));
	strncpy(if_idx.ifr_name, ifName, IFNAMSIZ-1);
	if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0)
	{
	    perror("SIOCGIFINDEX, Could not retrieve IF index...");
	    exit(-1);
	}
	/* Get the MAC address of the interface to send on */
	memset(&if_mac, 0, sizeof(struct ifreq));
	strncpy(if_mac.ifr_name, ifName, IFNAMSIZ-1);
	if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) < 0)
	{
	    perror("SIOCGIFHWADDR, Could not retrieve MAC address...");
	    exit(-1);
	}

	/* Construct the Ethernet header */
	memset(sendbuf, 0, BUF_SIZ);
	/* Ethernet header */
	memcpy(eh->h_source, &(if_mac.ifr_hwaddr.sa_data), ETH_ALEN);
	memcpy(eh->h_dest, ETH_ADDR_S1A, ETH_ALEN);
	/* Ethertype field */
	eh->h_proto = lsap;

	/* LLC header */
	printf("*** dsap = %d, eth_h = %p, llc_h = %p, d = %d\n"
				, htons(GN_ETHTYPE), eh, llc_header
				, (int)((void *)llc_header - (void *)eh));
	printf("*** LEN__ETH_HEADER = %d, LEN__HEADERS = %d\n"
				, LEN__ETH_HEADER, LEN__HEADERS);
	llc_header->dsap = lsap;
	llc_header->ssap = lsap;

	/* Packet data */
	int i = 0;
	for ( i = 0; i < LEN__DATA; i++ )
		{ data[i] = i; }

	printf(">>>>> Packet created:\n");
	print_hex_data(sendbuf, LEN__FRAME);
	printf("\n");

	/* Send packet */
	struct sockaddr_ll socket_address;

	socket_address.sll_family 	= PF_PACKET;
	socket_address.sll_ifindex 	= if_idx.ifr_ifindex;
	socket_address.sll_protocol	= lsap;
	/*ARP hardware identifier is ethernet*/
	socket_address.sll_hatype   = ARPHRD_ETHER;
	/*target is another host*/
	socket_address.sll_pkttype  = PACKET_OTHERHOST;

#ifndef USE_WRITE
	socket_address.sll_halen = ETH_ALEN;
	memcpy((void*)(socket_address.sll_addr), (void*)ETH_ADDR_S1A, ETH_ALEN);
#endif

	int b_written = 0;

#ifdef USE_WRITE

	if (	bind(sockfd, (struct sockaddr*)&socket_address,
							sizeof(struct sockaddr_ll)) < 0		)
	{
	    perror("Could not bind socket...");
		fprintf(stderr, "ERRNO = %d\n", errno);
	    exit(-1);
	}

	if ( ( b_written = write(sockfd, sendbuf, LEN__FRAME) ) < 0 )

#else

	if ( ( b_written = sendto(	sockfd,
								sendbuf, LEN__FRAME, 0,
								(struct sockaddr*)&socket_address,
								sizeof(struct sockaddr_ll))	) < 0	)

#endif

	{
		perror("Could not write socket...");
		fprintf(stderr, "ERRNO = %d\n", errno);
		exit(-1);
	}
	printf("Packet sent!, Bytes written = %d\n", b_written);

	return 0;
}

#define BYTES_PER_LINE 14	/*!< Number of bytes per line to be printed. */

/* print_eth_data */
int print_hex_data(const char *buffer, const int len)
{

	int last_byte = len - 1;

	if ( len < 0 )
		{ return(-1); }

	int i = 0;

	for ( i = 0; i < len; i++ )
	{
		if ( ( i % BYTES_PER_LINE ) == 0 )
			{ printf("\n\t\t\t"); }
		printf("%02X", 0xFF & (unsigned int)buffer[i]);
		if ( i < last_byte ) { printf(":"); }
	}

	return(0);

}
