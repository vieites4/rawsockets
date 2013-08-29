/*
 * @file ieee8023_frame.c
 * @author Ricardo Tubío (rtpardavila[at]gmail.com)
 * @version 0.1
 *
 * @section LICENSE
 *
 * This file is part of linklayertool.
 * linklayertool is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * linklayertool is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with linklayertool.  If not, see <http://www.gnu.org/licenses/>.
 */
 
 #include "ieee8023_frame.h"
//const unsigned char ETH_ADDR_ETH0[ETH_ALEN]={0x00, 0x23, 0x8b, 0xfc, 0x0e, 0x3b};
/*!< Ethernet NULL address. */
/* new_ethhdr */
struct ethhdr *new_ethhdr()
{

	struct ethhdr *buffer = NULL;
	buffer = (struct ethhdr *)malloc(ETH_HLEN);
	memset(buffer, 0, ETH_HLEN);
	return(buffer);

}

/* new_ieee8023_frame */
ieee8023_frame_t *new_ieee8023_frame()
{

	ieee8023_frame_t *buffer = NULL;
	buffer = (ieee8023_frame_t *)malloc(LEN__IEEE8023_FRAME);
	memset(buffer, 0, LEN__IEEE8023_FRAME);
	return(buffer);

}

/* init_ieee8023_frame */
ieee8023_frame_t *init_ieee8023_frame
	(	const int ll_sap,
		const unsigned char *h_source, const unsigned char *h_dest	)
{

	ieee8023_frame_t *f = new_ieee8023_frame();

	if ( set_ll_frame(&f->info, TYPE_IEEE_8023, ETH_FRAME_LEN) < 0 )
		{ log_app_msg("Could not set info adequately!\n"); }

	f->buffer.header.h_proto = htons(ETH_P_ALL);//ll_sap;//cambio
	memcpy(f->buffer.header.h_dest, h_dest, ETH_ALEN);
	memcpy(f->buffer.header.h_source, h_source, ETH_ALEN);

	return(f);

}

#ifdef KERNEL_RING

/* read_ieee8023_frame */
int read_ieee8023_frame(const void *rx_ring, ieee8023_frame_t *rx_frame)
{
		//struct tpacket_hdr *header = NULL;

		//header = (void *) rx_ring + (rxring_offset * getpagesize());

		log_app_msg("Unsupported mmap reading...\n");
		return(EX_ERR);
}

#else

/* ieee8023_frame_rx_cb */
void ieee8023_frame_rx_cb(const public_ev_arg_t *arg)
{

	ieee8023_frame_t *f = (ieee8023_frame_t *)arg->buffer;

	if ( read_ieee8023_frame(arg->socket_fd, f) < 0 )
	{
		log_app_msg("Could not read IEEE 802.3 frame.\n");
		return;
	}

	if ( print_ieee8023_frame(f) < 0 )
	{
		log_app_msg("Could not print IEEE 802.3 frame.\n");
		return;
	}

}

/* read_ieee8023_frame */
int read_ieee8023_frame(const int socket_fd, ieee8023_frame_t *frame)
{

	int b_read = read(socket_fd, &frame->buffer, ETH_FRAME_LEN);

	if ( b_read <= 0 )
	{
		log_sys_error("Could not read socket");
		return(EX_ERR);
	}

	if ( b_read < frame->info.frame_len )
	{
		log_app_msg("Read %d bytes, but %d bytes were requested.\n"
						, b_read, ETH_FRAME_LEN);
	}

	if ( set_ll_frame(&frame->info, TYPE_IEEE_8023, b_read) < 0 )
	{
		log_app_msg("Error setting ll_frame's info.\n");
	}

	return(EX_OK);

}

#endif

/* ieee8023_frame_tx_cb */
void ieee8023_frame_tx_cb(const public_ev_arg_t *arg)
{

	if ( __tx_ieee8023_test_frame
				(arg->socket_fd, arg->ll_sap, arg->if_index, arg->if_mac) < 0 )
	{
		log_app_msg("Could not transmit IEEE 802.3 frame.\n");
		return;
	}

	log_app_msg("Sleeping for %d (usecs)...\n", arg->tx_delay);
	if ( usleep(arg->tx_delay) < 0 )
	{
		log_app_msg("Could not sleep for %d.\n", arg->tx_delay);
		return;
	}

}

/* __tx_ieee8023_test_frame */
int __tx_ieee8023_test_frame
	(	const int socket_fd, const int ll_sap, const int if_index,
		const unsigned char *h_source	)
{

	ieee8023_frame_t *tx_frame
		= init_ieee8023_frame(ll_sap, h_source, ETH_ADDR_BROADCAST);
	tx_frame->info.frame_len = ETH_HLEN + 10;

	if ( print_ieee8023_frame(tx_frame) < 0 )
	{
		log_app_msg("Frame formatted incorrectly!\n");
		return(EX_ERR);
	}

	struct sockaddr_ll socket_address;
	socket_address.sll_ifindex = if_index;
	/* Address length*/
	socket_address.sll_halen = ETH_ALEN;
	/* Destination MAC */
	memcpy(socket_address.sll_addr, ETH_ADDR_BROADCAST, ETH_ALEN);

printf("%d\n",tx_frame->info.frame_len);
		int b_written = sendto(	socket_fd,&tx_frame->buffer, tx_frame->info.frame_len, 0,(struct sockaddr *)&socket_address,sizeof(struct sockaddr_ll)	);


	if ( b_written < 0 )
	{
		log_sys_error("Frame could not be sent");
		return(EX_SYS);
	}

	if ( b_written < ETH_FRAME_LEN )
	{
		log_sys_error("Could not transmit all bytes as requested");
		return(EX_SYS);
	}

	return(EX_OK);

}

/* print_ieee8023_frame */
int print_ieee8023_frame(const ieee8023_frame_t *frame)
{

	if ( print_ll_frame(&frame->info) < 0 ) { return(EX_ERR); }

	log_app_msg("\t* header->dst = ");
		print_eth_address(frame->buffer.header.h_dest);
		log_app_msg("\n");
	log_app_msg("\t* header->src = ");
		print_eth_address(frame->buffer.header.h_source);
		log_app_msg("\n");
	log_app_msg("\t* header->sap = %02X\n", frame->buffer.header.h_proto);

	int data_len = frame->info.frame_len - ETH_HLEN;
	log_app_msg("\t* data[%d] = ", data_len);

	if ( print_hex_data((char *)&frame->buffer.data, data_len) < 0 )
		{ log_app_msg("\n"); return(EX_ERR); }
	log_app_msg("\n");

	return(EX_OK);

}
