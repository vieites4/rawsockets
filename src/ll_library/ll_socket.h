/*
 * @file ll_socket.h
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

#ifndef LL_SOCKET_H_
#define LL_SOCKET_H_

#include "execution_codes.h"
#include "logger.h"
#include "ll_library/ll_frame.h"
#include "ll_library/ieee8023_frame.h"
#include "ll_library/ieee80211_frame.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <net/ethernet.h> /* the L2 protocols */
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <ev.h>

#ifdef KERNEL_RING
	#define FRAMES_PER_BLOCK 128		/*!< Number of frames per block. */
	#define NO_BLOCKS 1					/*!< Total number of blocks of the ring. */
#endif

/**************************************************************** DATA TYPES */

typedef struct ifreq ifreq_t;			/*!< Data type definition for ifreq. */
#define LEN__IFREQ sizeof(ifreq_t)		/*!< Length of ifreq structure. */

typedef struct sockaddr_ll sockaddr_ll_t;		/*!< Type for sockaddr_ll. */
#define LEN__SOCKADDR_LL sizeof(sockaddr_ll_t)	/*!< Length of sockaddr_ll. */

typedef	struct packet_mreq packet_mreq_t;		/*!< Type for packet_mreq. */
#define LEN__PACKET_MREQ sizeof(packet_mreq_t)	/*!< Length of packet_mreq. */

typedef struct tpacket_req tpacket_req_t;		/*!< Type for tpacket_req. */
#define LEN__TPACKET_REQ sizeof(tpacket_req_t)	/*!< Length of tpacket_req. */

/*!
	\struct ll_socket_t
	\brief Structure with the information for handling the ll_socket.
*/
typedef struct ll_socket
{

	#ifdef KERNEL_RING

		int tx_socket_fd;				/*!< FD of the tx socket. */	
		void *tx_ring_buffer;			/*!< Kernel mmap()ed tx ring. */
		sockaddr_ll_t *tx_ring_addr;	/*!< Address for the tx ring. */
		int tx_ring_len;				/*!< Length of the tx ring. */
	
		int rx_socket_fd;				/*!< FD of the rx socket. */	
		void *rx_ring_buffer;			/*!< Kernel mmap()ed tx ring. */
		sockaddr_ll_t *rx_ring_addr;	/*!< Address for the rx ring. */
		int rx_ring_len;				/*!< Length of the rx ring. */

	#else

		int socket_fd;					/*!< FD of the socket. */
		sockaddr_ll_t *addr;			/*!< TX address. */

		ll_frame_t *buffer;				/*!< Buffer for frames reception. */

	#endif
	
	ev_cb_t cb_frame_rx;					/*!< Callback frame rx function. */
	ev_cb_t cb_frame_tx;					/*!< Callback frame tx function. */

	struct ev_loop *loop;					/*!< Default event loop. */
	struct ev_io *rx_watcher;				/*!< Frame rx watcher. */
	struct ev_io *tx_watcher;				/*!< Frame tx watcher. */

	int state;					/*!< Function state of the ll_socket. */
	
	int ll_sap;					/*!< Link layer level Service Access Point. */
	char if_name[IF_NAMESIZE];	/*!< Name of the link layer level if. */
	int if_index;				/*!< Index of the link layer level if.*/
	char if_mac[ETH_ALEN];		/*!< MAC address of the link layer level if. */

	int tx_delay;				/*!< Delay (ms) between two test frames. */
	int frame_type;				/*!< Frame type for post-processing. */

} ll_socket_t;

#define LEN__LL_SOCKET 	sizeof(ll_socket_t)

/*!
 * \struct ev_io_arg
 * \brief Structure for passing some arguments to libev's io callback.
 */
typedef struct ev_io_arg
{

	struct ev_io watcher;			/*!< Watcher for the event. */

	void (*cb_frame_rx) (public_ev_arg_t *arg);		/*!< Callback frame rx. */
	void (*cb_frame_tx) (public_ev_arg_t *arg);		/*!< Callback frame tx. */

	public_ev_arg_t public_arg;		/*!< Data for external callbacks. */

} ev_io_arg_t;

#define LEN__EV_IO_ARG sizeof(ev_io_arg_t)

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// STRUCTURES MANAGEMENT
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/*!
	\brief Allocates memory for an ifreq structure. 
	\return A pointer to the newly allocated block of memory.
*/
ifreq_t *new_ifreq();

/*!
	\brief Allocates memory for an sockaddr_ll structure. 
	\return A pointer to the newly allocated block of memory.
*/
sockaddr_ll_t *new_sockaddr_ll();

/*!
	\brief Allocates memory for an packet_mreq structure. 
	\return A pointer to the newly allocated block of memory.
*/
packet_mreq_t *new_packet_mreq();

/*!
	\brief Allocates memory for an tpacket_req structure. 
	\return A pointer to the newly allocated block of memory.
*/
tpacket_req_t *new_tpacket_req();

/*!
	\brief Allocates memory for an ev_io_arg structure.
	\return A pointer to the newly allocated block of memory.
*/
ev_io_arg_t *new_ev_io_arg();

/*!
	\brief Allocates memory for a buffer that holds a MAC address.
	\return A pointer to the newly allocated block of memory.
*/
unsigned char *new_mac_buffer();

/*!
	\brief Initializes a new ev_io_tx_arg structure.
	\return A pointer to the newly allocated block of memory.
*/
ev_io_arg_t *init_ev_io_arg(ll_socket_t *ll_socket);

#ifdef KERNEL_RING

/*!
	\brief Initializes a tpacket structure for the request of a ring to the 
			kernel, with 1 block of memory containing the requested number 
			of frames.
	\param frames_per_block Number of frames for each of the blocks.
	\param no_blocks Number of blocks to be used for this ring.
	\return The initialized tpacket_req structure.
*/
tpacket_req_t *init_tpacket_req
					(const int frames_per_block, const int no_blocks);

#endif

/*!
	\brief Allocates memory for a socket_addr structure and fills it up with 
			the data necessary for defining the socket access to the kernel 
			ring through mmap().
	\param ll_socket The socket to be used.
	\return The initialized sockaddr_ll structure.
*/
sockaddr_ll_t *init_sockaddr_ll(const ll_socket_t *ll_socket,bool is_transmitter);

/*!
	\brief Gets the index of the given link layer level interface from the given
			link layer interface name.
	\param socket_fd Identifier of the socket.
	\param if_name The name of the link layer level interface.
	\return The index of the link layer level interface ( >= 0 ), otherwise, the
			identifier of the problem occurred ( < 0 );
*/
int if_name_2_if_index(const int socket_fd, const char *if_name);

/*!
	\brief Gets the MAC address of the given interface.
	\param socket_fd Identifier of the socket.
	\param if_name The name of the link layer level interface.
	\return EX_OK if the execution was correct, <0 otherwise.
*/
int get_mac_address
	(const int socket_fd, const char *if_name, unsigned char *mac);

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// LL_SOCKET MANAGEMENT
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/*!
	\brief Allocates memory for an ll_socket_info structure. 
	\return A pointer to the newly allocated block of memory.
*/
ll_socket_t *new_ll_socket();

/*!
	\brief Creates a RAW socket with TX and RX buffers initialized and mmap()ed
			to the appropriate kernelspace memory.
	\param is_transmitter Flag that indicates whether the socket must
							transmit test frames within its own loop.
	\param tx_delay Delay (in ms) after each test frame sent to the channel.
	\param ll_if_name Name of the link layer level interface to be used.
	\param ll_sap Service access point to be used.
	\return Structure containing all information necessary for handling this
			socket. A 'NULL' value indicates that an unsupported error has
			ocurred.
*/
ll_socket_t *init_ll_socket
	(	const bool is_transmitter, const int tx_delay,
		const char *ll_if_name, const int ll_sap,
		const int frame_type	);

/*!
	\brief Opens a new socket without binding it.
	\param is_transmitter Flag that indicates whether the socket must
							transmit test frames within its own loop.
	\param tx_delay Delay (in ms) after each test frame sent to the channel.
	\param ll_if_name Name of the link layer level interface.
	\param ll_sap Link layer service access point.
	\param frame_type Selects the type of frame to be managed.
	\return Socket information structure or NULL if a problem occurred.
*/
ll_socket_t *open_ll_socket
	(	const bool is_transmitter, const int tx_delay,
		const char* ll_if_name, const int ll_sap,
		const int frame_type	);

/*!
	\brief Creates and binds a new socket to the given SAP of the link layer.
	\param ll_socket Information of the socket to be created.
	\return Function execution result code.
*/
int bind_ll_socket(ll_socket_t *ll_socket,bool is_transmitter);

/*!
	\brief Closes the just created link layer level socket.
	\param ll_socket The socket to be closed.
	\return EX_OK if the socket could be closed correctly, <0 otherwise.
*/
int close_ll_socket(const ll_socket_t *ll_socket);

/*!
	\brief Sets the address for the given socket.
	\param ll_socket The ll_socket whose address are to be set.
	\return EX_OK if the socket could be closed correctly, <0 otherwise.
*/
int set_sockaddr_ll(ll_socket_t *ll_socket,bool is_transmitter);

/*!
	\brief Set the given link layer level socket in promiscuous mode.
	\param ll_socket The socket to be put in promiscuous mode.
	\return '0' in case the operation was correct, otherwise, a different 
			number.
	TODO Tests have not been performed over this function.
*/
int set_promiscuous_ll_socket(const ll_socket_t *ll_socket);

#ifdef KERNEL_RING

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// KERNEL RING
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/*!
	\brief Initializes both TX and RX rings for the given socket.
	\param ll_socket Socket whose rings are to be initialized.
	\return Function execution result code.
*/
int init_rings(ll_socket_t *ll_socket);

/*!
	\brief Initializes one of the rings with the given type.
	\param socket_fd FD of the socket whose ring is to be initialized.
	\param type Type of ring to be created, either TX or RX.
	\param frames_per_block Number of frames for a given block of memory.
	\param no_blocks Number of blocks that the buffer is composed of.
	\param ring Pointer to the initialized ring buffer.
	\return Length of the ring (>0) in case operation was successfull, (<0) 
				otherwise.
*/
int init_ring(	const int socket_fd, const int type,
				const int frames_per_block, const int no_blocks,
				void **ring	);
				
/*!
	\brief Closes the access requested to kernel tx and rx rings.
	\param ll_socket The socket whose rings are to be closed.
	\return EX_OK if the rings could be closed correctly, <0 otherwise.
*/
int close_rings(const ll_socket_t *ll_socket);

#endif

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// LIBEV
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/*!
	\brief Initializes libev for its usage with this ll_socket library.
	\param bool Flag that indicates whether the socket shall also transmit
				test frames within its own loop.
	\param ll_socket The ll_socket whose resources for the usage of the
						libev library are to be created.
	\return EX_OK in case of a correct execution, <0 otherwise.
*/
int init_events(const bool is_transmitter, ll_socket_t *ll_socket);

/*!
 * \brief Initializes the callback functions for the events to be registered.
 * \param ll_socket Structure with the information of the socket.
 */
int init_events_cb(ll_socket_t *ll_socket);

/*!
 * \brief Initializes the callback functions for the rx events to be registered.
 * \param ll_socket Structure with the information of the socket.
 */
int init_rx_events(ll_socket_t *ll_socket);

/*!
 * \brief Initializes the callback functions for the tx events to be registered.
 * \param ll_socket Structure with the information of the socket.
 */
int init_tx_events(ll_socket_t *ll_socket);

/*!
 * \brief Closes all resources related with the usage of the libev library.
 * \param ll_socket The ll_socket whose resources for the usage of the
 * 						libev library are to be closed.
 * \return EX_OK in case of a correct execution, <0 otherwise.
 */
int close_events(const ll_socket_t *ll_socket);

/*!
 * \brief Callback function for frames reception, <libev>.
 */
void cb_process_frame_rx
	(struct ev_loop *loop, struct ev_io *watcher, int revents);

#define WAIT_AFTER_TEST_TX 1000000 	/*!< ms after a successfull test tx. */

/*!
 * \brief Callback function for frames transmission, <libev>.
 */
void cb_process_frame_tx
	(struct ev_loop *loop, struct ev_io *watcher, int revents);

/*!
 * \brief Sets the function to be called whenever a frame is received.
 */
int set_cb_frame_rx(ll_socket_t *ll_socket, ev_cb_t cb_frame_rx);

/*!
 * \brief Sets the function to be called whenever a frame is to be transmitted.
 */
int set_cb_frame_tx(ll_socket_t *ll_socket, ev_cb_t cb_frame_tx);

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// DATA TX/RX INTERFACE
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

#define LL_SOCKET_STATE_UNDEF		-1		/*!< ll_socket not defined yet. */
#define LL_SOCKET_STATE_READY 		0		/*!< ll_socket ready to be used. */
#define LL_SOCKET_STATE_RUNNING 	1		/*!< ll_socket running */
#define LL_SOCKET_STATE_PAUSED		2		/*!< ll_socket not running */

int start_ll_socket(ll_socket_t *ll_socket);

#endif /* LL_SOCKET_H_ */
