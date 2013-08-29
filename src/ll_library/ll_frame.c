/*
 * @file ll_framebuffer.c
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

#include "ll_frame.h"

/*!< Ethernet broadcast address. */
const unsigned char ETH_ADDR_BROADCAST[ETH_ALEN]
                                    ={0x20, 0xaa, 0x4B, 0x12, 0x35, 0x96};//{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFf };//

//
//{ 0x00, 0x22, 0xfb, 0x8f, 0xe4, 0x9a }; //;00:23:8b:fc:0e:3b
//const unsigned char ETH_ADDR_ETH0[ETH_ALEN]={0x00, 0x23, 0x8b, 0xfc, 0x0e, 0x3b};
/*!< Ethernet NULL address. */
const unsigned char ETH_ADDR_NULL[ETH_ALEN]
                                    = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
/*!< Ethernet FAKE address. */
const unsigned char ETH_ADDR_FAKE[ETH_ALEN]
                                    = { 0x00,0x18,0x39,0xAE,0x7D,0xD5 };

/* new_ll_frame */
ll_frame_t *new_ll_frame()
{

	ll_frame_t *buffer = NULL;
	buffer = (ll_frame_t *)malloc(LEN__LL_FRAME);
	memset(buffer, 0, LEN__LL_FRAME);
	return(buffer);

}

/* new_ll_framebuffer */
ll_frame_t *new_ll_framebuffer()
{

	ll_frame_t *buffer = NULL;
	buffer = (ll_frame_t *)malloc(FRAMEBUFFER_LEN);
	memset(buffer, 0, FRAMEBUFFER_LEN);
	return(buffer);

}

/* init_ll_frame */
ll_frame_t *init_ll_frame
	(const int frame_type, const int frame_len)
{

	ll_frame_t *buffer = new_ll_frame();

	if ( set_ll_frame(buffer, frame_type, frame_len) < 0 )
	{
		log_app_msg("Frame %p incorrectly initialized.\n", (void *)buffer);
	}

	return(buffer);

}

/* set_ll_frame */
int set_ll_frame
	(ll_frame_t *frame, const int frame_type, const int frame_len)
{

	frame->frame_type = frame_type;
	frame->frame_len = frame_len;

	if ( gettimeofday(&frame->timestamp, NULL) < 0 )
	{
		log_sys_error("Cannot get timestamp");
		return(EX_ERR);
	}

	return(EX_OK);

}

/* print_ll_framebuffer */
int print_ll_frame(const ll_frame_t *frame)
{

	log_app_msg(">>>>> LL_FRAMEBUFFER:\n");
	log_app_msg("\t* type = %d\n", frame->frame_type);
	log_app_msg("\t* length (B) = %d\n", frame->frame_len);
	log_app_msg("\t* timestamp (usecs) = %lu\n", get_timestamp_usecs(frame));

	return(EX_OK);

}

/* get_timestamp_usecs */
uint64_t get_timestamp_usecs(const ll_frame_t *frame)
{
    return frame->timestamp.tv_sec * (uint64_t) 1000000
    				+ frame->timestamp.tv_usec;
}

/* print_eth_data */
int print_hex_data(const char *buffer, const int len)
{

	int last_byte = len - 1;

	if ( len < 0 )
		{ return(EX_WRONG_PARAM); }

	for ( int i = 0; i < len; i++ )
	{
		if ( ( i % BYTES_PER_LINE ) == 0 )
			{ log_app_msg("\n\t\t\t"); }
		log_app_msg("%02X", 0xFF & (unsigned int)buffer[i]);
		if ( i < last_byte ) { log_app_msg(":"); }
	}

	return(EX_OK);

}

/* print_eth_address */
void print_eth_address(const unsigned char *eth_address)
{

	printf("%02X:%02X:%02X:%02X:%02X:%02X",
  			(unsigned char) eth_address[0],
  			(unsigned char) eth_address[1],
  			(unsigned char) eth_address[2],
  			(unsigned char) eth_address[3],
  			(unsigned char) eth_address[4],
  			(unsigned char) eth_address[5]);

}
