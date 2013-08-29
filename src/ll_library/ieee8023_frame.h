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
 
#ifndef IEEE8023_FRAME_H_
#define IEEE8023_FRAME_H_

#include "execution_codes.h"
#include "logger.h"
#include "ll_library/ll_frame.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <sys/time.h>
#include <sys/socket.h>

//#define KERNEL_RING 1

#ifdef KERNEL_RING
	#include <linux/if_packet.h>
#endif

/****************************************************** IEEE 802.3 structures */

typedef struct ethhdr eth_header_t;		/*!< Data type definition for ethhdr. */

/*!
	\struct ieee8023_buffer
	\brief Structure for decoding an IEEE 802.3 ethernet frame.
 */
typedef struct ieee8023_frame_buffer
{

	eth_header_t header;		/*!< IEEE 802.3 header. */
	char data[ETH_DATA_LEN];	/*!< IEEE 802.3 data. */

} ieee8023_frame_buffer_t;

/*!
	\struct ieee8023_frame
	\brief Structure for holding an IEEE 802.3 frame with associated info.
 */
typedef struct ieee8023_frame
{

	ll_frame_t info;					/*!< Info of this frame. */
	ieee8023_frame_buffer_t buffer; 	/*!< Header + Data. */

} ieee8023_frame_t;

#define LEN__IEEE8023_FRAME sizeof(ieee8023_frame_t)

/******************************************************* IEEE 802.3 functions */

/*!
	\brief Allocates memory for an ethhdr structure, including MAX PAYLOAD.
	\return A pointer to the newly allocated block of memory.
*/
struct ethhdr *new_ethhdr();

/*!
	\brief Allocates memory for a ieee8023_buffer structure.
	\return A pointer to the newly allocated block of memory.
*/
ieee8023_frame_t *new_ieee8023_frame();

/*!
 * \brief Initializes an IEEE 802.3 frame with the given data.
 * \param ll_sap Link layer Service Access Point.
 * \param h_source Pointer to the MAC source address.
 * \param h_dest Pointer to the buffer that holds the MAC destination.
 * \return A pointer to the initialized structure.
 */
ieee8023_frame_t *init_ieee8023_frame
	(	const int ll_sap,
		const unsigned char *h_source, const unsigned char *h_dest	);

/*!
 * \brief Callback function to be called whenever an IEEE 802.3 frame is
 * 			received.
 * \param arg Argument given by the event handler.
 */
void ieee8023_frame_rx_cb(const public_ev_arg_t *arg);

/*!
 * \brief Callback function to be called whenever an IEEE 802.3 frame is
 * 			to be transmitted.
 * \param arg Argument given by the event handler.
 */
void ieee8023_frame_tx_cb(const public_ev_arg_t *arg);

#ifdef KERNEL_RING
	int read_ieee8023_frame(const void *rx_ring, ieee8023_frame_t *rx_frame);
	int __tx_ieee8023_test_frame
		(const void *tx_ring, const int ll_sap, const unsigned char *h_source);
#else

	/*!
	 * \brief Reads from a socket an ll_framebuffer.
	 * \param socket_fd The socket from where to read the frame.
	 * \param rx_frame Structure for reading the IEEE 802.3 frame.
	 * \return EX_OK if everything was correct; otherwise < 0.
	 */
	int read_ieee8023_frame(const int socket_fd, ieee8023_frame_t *rx_frame);

	/*!
	 * \brief Writes to a socket an IEEE 802.3 frame, filled up with null data.
	 * \param socket_fd The socket where to write the frame.
	 * \return EX_OK if everything was correct; othewise < 0.
	 */
	int __tx_ieee8023_test_frame
		(	const int socket_fd, const int ll_sap, const int if_index,
			const unsigned char *h_source	);

#endif

/*!
 * \brief Prints the data of the given IEEE 802.3 frame.
 * \param frame The frame whose data is to be printed out.
 * \return EX_OK if everything was correct; otherwise < 0.
 */
int print_ieee8023_frame(const ieee8023_frame_t *frame);

#endif /* IEEE8023_FRAME_H_ */
