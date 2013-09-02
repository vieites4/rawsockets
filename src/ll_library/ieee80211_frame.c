/*
 * @file ieee80211_frame.c
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

 #include "ieee80211_frame.h"
const unsigned char AMINHA[ETH_ALEN]={ 0x00, 0x22, 0xfb, 0x8f, 0xe4, 0x9a }; //;00:23:8b:fc:0e:3b
/* new_ieee80211_frame */
ieee80211_frame_t *new_ieee80211_frame()
{
	printf("ini4\n");
	ieee80211_frame_t *buffer = NULL;
	buffer = (ieee80211_frame_t *)malloc(LEN__IEEE80211_FRAME);
	memset(buffer, 0, LEN__IEEE80211_FRAME);
	return(buffer);

}

/* init_ieee80211_frame */
/*ieee80211_frame_t *init_ieee80211_frame
	(	const uint8_t mac_service, const uint8_t flags,
		const uint16_t duration_id,
		const unsigned char *bssid,
		const unsigned char *h_source, const unsigned char *h_dest,
		const uint16_t sequence_control,
		const unsigned char *dist_address,
		const unsigned char *qos)
{

	ieee80211_frame_t *f = new_ieee80211_frame();
	//ieee80211_header_t *h = &f->buffer.header;

	if ( set_ll_frame(&f->info, TYPE_IEEE_80211, ETH_FRAME_LEN) < 0 )
		{ log_app_msg("Could not set info adequately!\n"); }

	f->buffer.header.frame_control.mac_service = mac_service;
	f->buffer.header.frame_control.flags = flags;

	f->buffer.header.duration_id = duration_id;

	//memcpy(h->bssid_address, bssid, ETH_ALEN);
	memcpy(f->buffer.header.bssid_address, bssid, ETH_ALEN);
	memcpy(f->buffer.header.dest_address, h_dest, ETH_ALEN);
	memcpy(f->buffer.header.src_address, h_source, ETH_ALEN);

	f->buffer.header.sequence_control = sequence_control;

	memcpy(f->buffer.header.dist_address, dist_address, ETH_ALEN);
     memcpy(f->buffer.header.qos, qos, 2);

	//falta meterlle o qos control

	return(f);

}*/



ieee80211_frame_t *init_ieee80211_frame	(	const int ll_sap, const unsigned char *h_source, const unsigned char *h_dest	)
{

	ieee80211_frame_t *f = new_ieee80211_frame();
	printf("ini3\n");
	if ( set_ll_frame(&f->info, TYPE_IEEE_80211, ETH_FRAME_LEN) < 0 )
		{ log_app_msg("Could not set info adequately!\n"); }

	//f->buffer.header.h_proto = htons(ETH_P_ALL);//ll_sap;//cambio
	memcpy(f->buffer.header.dest_address, h_dest, ETH_ALEN);
	memcpy(f->buffer.header.src_address, h_source, ETH_ALEN);

	return(f);

}

#ifdef KERNEL_RING

/* read_ieee80211_frame */
int read_ieee80211_frame(const void *rx_ring, ieee80211_frame_t *rx_frame)
{
		//struct tpacket_hdr *header = NULL;
	https://github.com/vieites4/rawsockets/
		//header = (void *) rx_ring + (rxring_offset * getpagesize());

		log_app_msg("Unsupported mmap reading...\n");
		return(EX_ERR);
}

#else

/* ieee80211_frame_rx_cb */
void ieee80211_frame_rx_cb(const public_ev_arg_t *arg)
{
printf("inicio\n");
	ieee80211_frame_t *f = (ieee80211_frame_t *)arg->buffer;

	if ( read_ieee80211_frame(arg->socket_fd, f) < 0 )
	{
		log_app_msg("Could not read IEEE 802.11 frame.\n");
		return;
	}

	if ( print_ieee80211_frame(f) < 0 )
	{
		log_app_msg("Could not print IEEE 802.11 frame.\n");
		return;
	}

}

/* read_ieee80211_frame */
int read_ieee80211_frame(const int socket_fd, ieee80211_frame_t *frame)
{
printf("read\n");
	int b_read = recvfrom(socket_fd, &frame->buffer,IEEE_80211_FRAME_LEN, 0,NULL,NULL);//);//ANTES POÑÍA read
	printf("ini2\n");
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

	if ( set_ll_frame(&frame->info, TYPE_IEEE_80211, b_read) < 0 )
	{
		log_app_msg("Error setting ll_frame's info.\n");
	}

	return(EX_OK);

}

#endif

/* ieee80211_frame_tx_cb */
void ieee80211_frame_tx_cb(const public_ev_arg_t *arg)
{
printf("ini\n");
	if ( __tx_ieee80211_test_frame
				(arg->socket_fd, arg->ll_sap, arg->if_index, arg->if_mac) < 0 )
	{
		log_app_msg("Could not transmit IEEE 802.11 frame.\n");
		return;
	}

	log_app_msg("Sleeping for %d (usecs)...\n", arg->tx_delay);
	if ( usleep(arg->tx_delay) < 0 )
	{
		log_app_msg("Could not sleep for %d.\n", arg->tx_delay);
		return;
	}

}

/* __tx_ieee80211_test_frame */
int __tx_ieee80211_test_frame
	(	const int socket_fd, const int ll_sap, int if_index,
		const unsigned char *h_source	)
{const unsigned char QOS_M[2]={0x00,0x00};const unsigned char RTH[2]={0x00,0x00};
//	ieee80211_frame_t *tx_frame	= init_ieee80211_frame(0x08, 0x04, 0, ETH_ADDR_BROADCAST, h_source, ETH_ADDR_FAKE,	0,ETH_ADDR_NULL,QOS_M);//null
//ieee80211_frame_t *tx_frame	= init_ieee80211_frame(0x08, 0x04, 0, ETH_ADDR_BROADCAST, h_source, ETH_ADDR_FAKE,	0,ETH_ADDR_NULL,QO);//null
ieee80211_frame_t *tx_frame = init_ieee80211_frame(ll_sap, h_source, AMINHA);//ETH_ADDR_BROADCAST);

tx_frame->info.frame_len = IEEE_80211_HLEN + 10;
	//printf("%x %x \n",* (h_source+5),* (h_source +6));
	//memcpy(tx_frame->buffer.rth,RTH,2);
//const unsigned char FCS_M[4]={ 0xD4,0x52,0x0E,0x1E };
	memcpy(tx_frame->buffer.data,"0xffffffffff",35); //así é como se introducen os datos
	//memcpy(tx_frame->buffer.fcs,FCS_M,4);

//meter o qos control xusto ó principio do data??
//meter o fcs dentro do data.
	if ( print_ieee80211_frame(tx_frame) < 0 )
	{
		log_app_msg("Frame formatted incorrectly!\n");
		return(EX_ERR);
	}
	struct sockaddr_ll socket_address;
	/* Address length*/

	socket_address.sll_halen = ETH_ALEN;
	socket_address.sll_family   = PF_PACKET;	// engadido
	socket_address.sll_protocol = htons(ETH_P_ALL);	// htons(0x0707);//engadido
#define ARPHRD_IEEE80211 801
    socket_address.sll_hatype= ARPHRD_IEEE80211;
	socket_address.sll_ifindex  = if_index;//if_nametoindex(if_index);
//printf("%x\n",socket_address.sll_protocol);->0x300
	//printf("%x\n",ll_sap);-->0x8c
	/* Destination MAC */
	memcpy(socket_address.sll_addr,AMINHA , ETH_ALEN);//ETH_ADDR_BROADCAST
	//int b_written = write(socket_fd, tx_frame, ETH_FRAME_LEN);
	int b_written = sendto(	socket_fd,&tx_frame->buffer , tx_frame->info.frame_len,0,(struct sockaddr *)&socket_address,sizeof(struct sockaddr_ll)	);//&tx_frame->buffer //(struct sockaddr *)&socket_address
	if ( b_written < 0 )
	{
		log_sys_error("Frame could not be sent");
		return(EX_SYS);
	}

	if ( b_written < 10)//ETH_FRAME_LEN )
	{
		log_sys_error("Could not transmit all bytes as requested");
		return(EX_SYS);
	}

	return(EX_OK);

}

/* print_ieee80211_frame */
int print_ieee80211_frame(const ieee80211_frame_t *frame)
{

	if ( print_ll_frame(&frame->info) < 0 ) { return(EX_ERR); }

//	log_app_msg("\t* header->frame_control.mac_service = %02X\n"
	//						, frame->buffer.header.frame_control.mac_service);
	//	log_app_msg("\t* header->frame_control.flags = %02X\n"
	//				, frame->buffer.header.frame_control.flags);
	//log_app_msg("\t* header->duration_id = %d\n"
	//				, frame->buffer.header.duration_id);

	//	log_app_msg("\t* header->bssid = ");
	//print_eth_address(frame->buffer.header.bssid_address);
	//log_app_msg("\n");
	log_app_msg("\t* header->src = ");
		print_eth_address(frame->buffer.header.src_address);
		log_app_msg("\n");
	log_app_msg("\t* header->dest = ");
		print_eth_address(frame->buffer.header.dest_address);
		log_app_msg("\n");

		//log_app_msg("\t* header->sequence_control = %d\n"
		//			, frame->buffer.header.sequence_control);

		//log_app_msg("\t* header->dist = ");
		//print_eth_address(frame->buffer.header.dist_address);
		//log_app_msg("\n");

	int data_len = frame->info.frame_len - IEEE_80211_HLEN;
	log_app_msg("\t* data[%d] = ", data_len);
	if ( print_hex_data((char *)&frame->buffer.data, data_len) < 0 )
		{ log_app_msg("\n"); return(EX_ERR); }
	log_app_msg("\n");

	return(EX_OK);

}
//mememe
