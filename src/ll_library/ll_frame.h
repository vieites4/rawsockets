/*
 * @file ll_frame.h
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

#ifndef LL_FRAME_H_
#define LL_FRAME_H_

#include "execution_codes.h"
#include "logger.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/time.h>
#include <linux/if_ether.h>

#include <ev.h>

/*********************************************************** DATA STRUCTURES */

/*!< Ethernet broadcast address. */
extern const unsigned char ETH_ADDR_BROADCAST[ETH_ALEN];
/*!< Ethernet NULL address. */
extern const unsigned char ETH_ADDR_NULL[ETH_ALEN];
/*!< Ethernet FAKE address. */
extern const unsigned char ETH_ADDR_FAKE[ETH_ALEN];

#define TYPE_BUFFER 		0	/*!< Defines a buffer with unknown data. */
#define TYPE_IEEE_8023 		1	/*!< Buffer with an IEEE 802.3 frame. */
#define TYPE_IEEE_80211 	2	/*!< Buffer with an IEEE 802.11 frame. */

/*!
	\struct ll_framebuffer
	\brief Structure for managing generic frames.
 */
typedef struct ll_frame
{

	int frame_type;				/*!< Type of the frame. */
	int frame_len;				/*!< Length of the total bytes read. */

	struct timeval timestamp;	/*!< Frame creation timestamp (usecs). */

} ll_frame_t;

#define LEN__LL_FRAME 		sizeof(ll_frame_t)
#define FRAMEBUFFER_LEN		LEN__LL_FRAME + 5000	/*!< Framebuffer len. */

/************************************************** Event handling structures */

#define LEN__EV_IO 		sizeof(struct ev_io)

/*!
 * \struct public_ev_arg
 * \brief Structure for holding public arguments to be passed to callback
 * 			functions.
 */
typedef struct public_ev_arg
{

#ifdef KERNEL_RING
	void *rx_ring;					/*!< Kernel RX_RING. */
#else
	int socket_fd;					/*!< Socket file descriptor. */
	ll_frame_t *buffer;				/*!< Buffer for frames reception. */
#endif

	int ll_sap;						/*!< Link layer SAP. */
	int tx_delay;					/*!< Delay (ms) between two test frames. */
	int if_index;					/*!< Index of the interface. */
	unsigned char if_mac[ETH_ALEN];	/*!< MAC of the link layer interface. */

} public_ev_arg_t;

#define LEN__PUBLIC_EV_ARG sizeof(public_ev_arg_t)

typedef void (*ev_cb_t)(public_ev_arg_t *);		/*!< Callback function. */

/****************************************************************** FUNCTIONS */

/*!
 * \brief Allocates memory for a ll_frame structure.
 * \return A pointer to the newly allocated block of memory.
 */
ll_frame_t *new_ll_frame();

/*!
 * \brief Allocates memory for a ll_frame structure.
 * \return A pointer to the newly allocated block of memory.
 */
ll_frame_t *new_ll_framebuffer();

/*!
 * \brief Initializes and allocates a new frame buffer with the given data.
 * 	\param frame_type Type of the frame contained in this buffer.
 * 	\param frame_len Length of the frame contained.
 * 	\return A pointer to the newly initialized block of memory.
 */
ll_frame_t *init_ll_frame(const int frame_type, const int frame_len);

/*!
 * \brief Initializes the given frame buffer with the given data.
 * 	\param frame_type Type of the frame contained in this buffer.
 * 	\param frame_len Length of the frame contained.
 * 	\return A pointer to the newly initialized block of memory.
 */
int set_ll_frame
	(ll_frame_t *frame, const int frame_type, const int frame_len);

#define BYTES_PER_LINE 8	/*!< Number of bytes per line to be printed. */

/*!
 * \brief Prints the given ll_framebuffer.
 * \param frame The ll_framebuffer to be printed.
 * \return EX_OK if everything was correct; otherwise < 0.
 * */
int print_ll_frame(const ll_frame_t *frame);

/*!
 * \brief Gets the timestamp of a given frame.
 * \param frame ll_framebuffer with the timestamp to be calculated.
 * \return Long number containing the timestamp of the given frame.
 */
uint64_t get_timestamp_usecs(const ll_frame_t *frame);

/*!
 * \brief Prints the data field of the given IEEE 802.3 frame.
 * \param buffer The IEEE 802.3 frame whose data is to be printed.
 * \return EX_OK if everything was correct; otherwise < 0.
 */
int print_hex_data(const char *buffer, const int len);

/*!
	\brief Prints the given Ethernet address.
	\param eth_address Ethernet address as an array.
 */
void print_eth_address(const unsigned char *eth_address);

#endif /* LL_FRAME_H_ */
