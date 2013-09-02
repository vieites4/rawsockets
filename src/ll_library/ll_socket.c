/*
 * @file ll_socket.c
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

#include "ll_socket.h"

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// STRUCTURES MANAGEMENT
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/* new_if_req */
ifreq_t* new_ifreq()
{
	ifreq_t* buffer = NULL;
	buffer = (ifreq_t *)malloc(LEN__IFREQ);
	return(buffer);
}

/* new_sockaddr_ll */
sockaddr_ll_t *new_sockaddr_ll()
{
	sockaddr_ll_t *buffer = NULL;
	buffer = (sockaddr_ll_t *)malloc(LEN__SOCKADDR_LL);
	memset(buffer, 0, LEN__SOCKADDR_LL);
	return(buffer);
}

/* new_packet_mreq */
packet_mreq_t *new_packet_mreq()
{
	packet_mreq_t *buffer = NULL;
	buffer = (packet_mreq_t *)malloc(LEN__PACKET_MREQ);
	memset (buffer, 0, LEN__PACKET_MREQ);
	return(buffer);
}

/* new_tpacket_req */
tpacket_req_t *new_tpacket_req()
{
	tpacket_req_t *buffer = NULL;
	buffer = (tpacket_req_t *)malloc(LEN__TPACKET_REQ);
	memset (buffer, 0, LEN__TPACKET_REQ);
	return(buffer);
}

/* new_ev_io_arg */
ev_io_arg_t *new_ev_io_arg()
{
	ev_io_arg_t *buffer = NULL;
	buffer = (ev_io_arg_t *)malloc(LEN__EV_IO_ARG);
	memset (buffer, 0, LEN__EV_IO_ARG);
	return(buffer);
}

/* new_mac_buffer */
unsigned char *new_mac_buffer()
{
	unsigned char *buffer = NULL;
	buffer = (unsigned char *)malloc(ETH_ALEN);
	memset (buffer, 0, ETH_ALEN);
	return(buffer);
}

/* init_ev_io_arg */
ev_io_arg_t *init_ev_io_arg(ll_socket_t *ll_socket)
{
	ev_io_arg_t *a = new_ev_io_arg();

	a->cb_frame_rx = ll_socket->cb_frame_rx;
	a->cb_frame_tx = ll_socket->cb_frame_tx;

	a->public_arg.ll_sap = ll_socket->ll_sap;
	a->public_arg.tx_delay = ll_socket->tx_delay;

	memcpy(a->public_arg.if_mac, ll_socket->if_mac, ETH_ALEN);

	#ifdef KERNEL_RING
		a->public_arg.rx_ring = ll_socket->rx_ring_buffer;
	#else
		a->public_arg.buffer = ll_socket->buffer;
	#endif
a->public_arg.if_index=ll_socket->if_index;


	return(a);
}

#ifdef KERNEL_RING

/* init_tpacket_req */
tpacket_req_t *init_tpacket_req
					(const int frames_per_block, const int no_blocks)
{
	tpacket_req_t *t = new_tpacket_req();
  	t->tp_block_size = frames_per_block * getpagesize();
  	t->tp_block_nr = no_blocks;
  	t->tp_frame_size = getpagesize();
  	t->tp_frame_nr = frames_per_block * no_blocks;
  	return(t); 	
}

#endif

/* init_sockaddr_ll */
sockaddr_ll_t *init_sockaddr_ll(const ll_socket_t* ll_socket, bool is_transmitter)
{

	sockaddr_ll_t *t = new_sockaddr_ll();
	
	t->sll_family = PF_PACKET;
	t->sll_ifindex = ll_socket->if_index;
	if (is_transmitter) {t->sll_protocol = htons(ll_socket->ll_sap);}else t->sll_protocol=htons(ETH_P_ALL);
  	t->sll_halen = ETH_ALEN;
  	memset(t->sll_addr, 0xff, ETH_ALEN);
  	


  	return(t);
  	
}

/* if_name_2_if_index */
int if_name_2_if_index(const int socket_fd, const char *if_name)
{
	printf("ini5\n");
	int len_if_name = -1;

	if ( if_name == NULL )
		{ return(EX_NULL_PARAM); }
	
	len_if_name = strlen(if_name);
	
	if ( len_if_name == 0 )
		{ return(EX_EMPTY_PARAM); }
	if ( len_if_name > IF_NAMESIZE )
		{ return(EX_WRONG_PARAM); }

	ifreq_t *ifr = new_ifreq();
	strncpy(ifr->ifr_name, if_name, len_if_name);
	
	if ( ioctl(socket_fd, SIOCGIFINDEX, ifr) < 0 )
		{ handle_sys_error("Could not get interface index"); }

	return(ifr->ifr_ifindex);

}

/* get_mac_address */
int get_mac_address
	(const int socket_fd, const char *if_name, unsigned char *mac)
{
	printf("ini6\n");
	int len_if_name = -1;

	if ( if_name == NULL )
		{ return(EX_NULL_PARAM); }
	if ( mac == NULL )
		{ return(EX_NULL_PARAM); }

	if ( socket_fd < 0 )
		{ return(EX_WRONG_PARAM); }

	len_if_name = strlen(if_name);

	if ( len_if_name == 0 )
		{ return(EX_EMPTY_PARAM); }
	if ( len_if_name > IF_NAMESIZE )
		{ return(EX_WRONG_PARAM); }

	ifreq_t *ifr = new_ifreq();
	strncpy(ifr->ifr_name, if_name, len_if_name);

	if ( ioctl(socket_fd, SIOCGIFHWADDR, ifr) < 0 )
	{
		log_sys_error("Could not get interface index");
		return(EX_SYS);
	}

	memcpy(mac, ifr->ifr_hwaddr.sa_data, ETH_ALEN);

	return(EX_OK);

}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// LL_SOCKET MANAGEMENT
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/* new_ll_socket */
ll_socket_t *new_ll_socket()
{
	ll_socket_t *buffer = NULL;
	buffer = (ll_socket_t *)malloc(LEN__LL_SOCKET);
	memset (buffer, 0, LEN__LL_SOCKET);
	return(buffer);
}

/* init_ll_socket */
ll_socket_t *init_ll_socket
	(	const bool is_transmitter, const int tx_delay,
		const char *ll_if_name, const int ll_sap,
		const int frame_type	)
{

	#ifdef KERNEL_RING
		int tx_socket_fd = -1, rx_socket_fd = -1;
	#else
		int socket_fd = -1;
	#endif
	int ll_if_index = -1;
	ll_socket_t *s = new_ll_socket();
	
	s->state = LL_SOCKET_STATE_UNDEF;

	// 1) create RAW socket(s)	
	#ifdef KERNEL_RING
		if ( ( tx_socket_fd = socket(AF_PACKET, SOCK_RAW, ll_sap) ) < 0 )
			{ handle_sys_error("Could not open TX socket"); }
		if ( ( rx_socket_fd = socket(AF_PACKET, SOCK_RAW, ll_sap) ) < 0 )
			{ handle_sys_error("Could not open RX socket"); }
	#else
		if ( ( socket_fd = socket(AF_PACKET, SOCK_RAW, ll_sap) ) < 0 )
			{ handle_sys_error("Could not open socket"); }
	#endif
/*
		int so_broadcast=1;
		int z = setsockopt(socket_fd,SOL_SOCKET,SO_BROADCAST,&so_broadcast,sizeof so_broadcast);
		printf("nome socket %d \n",socket_fd);

		if ( z )perror("setsockopt(2)");//If the setsockopt(2) function returns zero, the socket s has been enabled to perform broadcasting
		//printf(socket_fd);
*/
	
	// 2) initialize fields
	#ifdef KERNEL_RING
		s->tx_socket_fd = tx_socket_fd;
		s->rx_socket_fd = rx_socket_fd;
	#else
		s->socket_fd = socket_fd;
		s->buffer = new_ll_framebuffer();
	#endif

	s->ll_sap = ll_sap;
	s->frame_type = frame_type;
	s->tx_delay = tx_delay;

	#ifdef KERNEL_RING
		log_app_msg("Socket created, TX_FD = %d, RX_FD = %d, ll_sap = %d\n",
						tx_socket_fd, rx_socket_fd, ll_sap);
	#else
		log_app_msg("Socket created, FD = %d, ll_sap = %d\n",
						socket_fd, ll_sap);
	#endif
	
	// 3) get interface index from interface name
	#ifdef KERNEL_RING
		int socket_fd = tx_socket_fd;
	#endif
	//if ( ( ll_if_index = if_name_2_if_index(socket_fd, ll_if_name) ) < 0 ) //cambio
		if ((ll_if_index=if_nametoindex(ll_if_name))<0)
		{ handle_app_error("Could not get index, if_name = %s\n", ll_if_name); }
	//
	strncpy(s->if_name, ll_if_name, strlen(ll_if_name));
	s->if_index = ll_if_index;
	//printf("%s %d\n", ll_if_name,s->if_index);
	//int j=1;
	//printf(ll_if_index);
	
	// 4) get interface MAC address from interface name
	//if (is_transmitter){
	if ( get_mac_address
			(socket_fd, ll_if_name, (unsigned char *)s->if_mac) < 0 )
	{
		handle_app_error(	"Could not get MAC address, if_name = %s\n"
							, ll_if_name	);
	}//}

	log_app_msg("IF: name = %s, index = %d, MAC = ", ll_if_name, ll_if_index);
		print_eth_address((unsigned char *)s->if_mac);
		log_app_msg("\n");

	// 5) initialize events
	if ( init_events(is_transmitter, s) < 0 )
		{ handle_app_error("Could not initialize event manager!"); }
printf("volvo de init_events\n");
print_eth_address(s->if_mac);
	// Ready is socket's final state
	s->state = LL_SOCKET_STATE_READY;

	return(s);
	
}

/* open_ll_socket */
ll_socket_t *open_ll_socket
	(	const bool is_transmitter, const int tx_delay,
		const char* ll_if_name, const int ll_sap,
		const int frame_type	)
{

	// 1) create RAW socket
	ll_socket_t *ll_socket = init_ll_socket
			(is_transmitter, tx_delay, ll_if_name, ll_sap, frame_type);
printf("volvo de init_ll_socket\n");
//print_eth_address(ll_socket->if_mac);
	#ifdef KERNEL_RING

	// 2) initialize rings for frames tx+rx
	if ( init_rings(ll_socket) < 0 )
		{ handle_app_error("Could not initialize TX/RX rings.\n"); }
	log_app_msg("IO rings iniatialized.\n");

	#endif
	
	// 3) bind RAW socket	

	if ( bind_ll_socket(ll_socket,is_transmitter) < 0 )
		{ handle_sys_error("Could not bind socket"); }

	log_app_msg("ll_socket bound, ll_sap = %d.\n", ll_socket->ll_sap);

	return(ll_socket);

}

/* bind_socket */
int bind_ll_socket(ll_socket_t *ll_socket, bool is_transmitter)
{

	sockaddr_ll_t *sll = init_sockaddr_ll(ll_socket,is_transmitter); //o interesante sería que solo se aplicara o protocolo no caso da transmisión, non da lectura
	
	#ifdef KERNEL_RING

		if ( bind(	ll_socket->tx_socket_fd,
					(struct sockaddr *)sll, LEN__SOCKADDR_LL)
				< 0 )
			{ handle_sys_error("Binding TX socket"); }
	
		if ( bind(	ll_socket->rx_socket_fd,
					(struct sockaddr *)sll, LEN__SOCKADDR_LL)
				< 0 )
			{ handle_sys_error("Binding RX socket"); }

	#else

		if ( bind(	ll_socket->socket_fd,
					(struct sockaddr *)sll, LEN__SOCKADDR_LL)
				< 0 )
			{ handle_sys_error("Binding socket"); }

	#endif
	
	return(EX_OK);

}

/* close_ll_socket */
int close_ll_socket(const ll_socket_t *ll_socket)
{

	int result = EX_OK;

	#ifdef KERNEL_RING
		if ( close_rings(ll_socket) < 0 )
		{
			log_sys_error("Error closing rings");
			result = EX_ERR;
		}
	#endif
	#ifdef KERNEL_RING
		if ( close(ll_socket->tx_socket_fd) < 0 )
		{
			log_sys_error("Closing TX socket");	
			result = EX_ERR;
		}

		if ( close(ll_socket->rx_socket_fd) < 0 )
		{
			log_sys_error("Closing RX socket");	
			result = EX_ERR;
		}
	#else
		if ( close(ll_socket->socket_fd) < 0 )
		{
			log_sys_error("Closing socket");	
			result = EX_ERR;
		}
	#endif

	if ( close_events(ll_socket) < 0 )
	{
		log_sys_error("Closing events manager");
		result = EX_ERR;
	}

	return(result);

}

/* set_sockaddr_ll */
int set_sockaddr_ll(ll_socket_t *ll_socket, bool is_transmitter)
{
bool is_true=1;
	#ifdef KERNEL_RING
		ll_socket->tx_ring_addr = init_sockaddr_ll(ll_socket,is_true);
		ll_socket->rx_ring_addr = init_sockaddr_ll(ll_socket,not(is_true));
	#else
		ll_socket->addr = init_sockaddr_ll(ll_socket,is_transmitter);
	#endif
	
	return(EX_OK);
	
}

/* set_promiscuous_ll_socket */
int set_promiscuous_ll_socket(const ll_socket_t *ll_socket)
{

	packet_mreq_t *mr = new_packet_mreq();
	
	mr->mr_ifindex = ll_socket->if_index;
	mr->mr_type = PACKET_MR_PROMISC;
	

	#ifdef KERNEL_RING
	if ( setsockopt(	ll_socket->rx_socket_fd,
						SOL_PACKET, PACKET_ADD_MEMBERSHIP,
						mr, LEN__PACKET_MREQ	) < 0 )
	#else
	if ( setsockopt(	ll_socket->socket_fd,
						SOL_PACKET, PACKET_ADD_MEMBERSHIP,
						mr, LEN__PACKET_MREQ	) < 0 )
	#endif
		{ handle_sys_error("Could not set promiscuous mode"); }

	return(EX_OK);
	
}

#ifdef KERNEL_RING

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// KERNEL RING
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/* init_rings */
int init_rings(ll_socket_t *ll_socket)
{
  	
	// 1) initialize rx ring
	if ( ( ll_socket->rx_ring_len
			= init_ring(	ll_socket->rx_socket_fd, PACKET_RX_RING,
							FRAMES_PER_BLOCK, NO_BLOCKS,
							&ll_socket->rx_ring_buffer	) )
				< 0 )
	{
  		log_app_msg("Could not set initialize RX ring.");
  		return(EX_ERR);
	}
	
	// 2) initialize tx ring
	if ( ( ll_socket->tx_ring_len
			= init_ring(	ll_socket->tx_socket_fd, PACKET_TX_RING,
							FRAMES_PER_BLOCK, NO_BLOCKS,
							&ll_socket->tx_ring_buffer	) )
				< 0 )
	{
  		log_app_msg("Could not set initialize TX ring.");
  		return(EX_ERR);
	}
	
  	// 3) set destination address for both kernel rings
  	if ( set_sockaddr_ll(ll_socket,0) < 0 ) //puxen 0 por poñer algo
  	{
  		log_app_msg("Could not set sockaddr_ll for TX/RX rings.");
  		return(EX_ERR);
  	}
  	
	return(EX_OK);

}

/* init_ring */
int init_ring(	const int socket_fd, const int type,
				const int frames_per_block, const int no_blocks,
				void **ring	)
{

	int ring_access_flags = PROT_READ | PROT_WRITE;
	tpacket_req_t *p = init_tpacket_req(frames_per_block, no_blocks);
	int ring_len = ( p->tp_block_size ) * ( p->tp_block_nr );
  	
  	// 1) export kernel mmap()ed memory
  	if ( setsockopt(socket_fd, SOL_PACKET, type, p, LEN__TPACKET_REQ) < 0 )
	{
		log_sys_error("Setting socket options for this ring");
		return(EX_ERR);
	}

	#ifdef TPACKET_V2
  		int val = TPACKET_V1;
  		if ( setsockopt(socket_fd, SOL_PACKET, PACKET_HDRLEN,
  							&val, sizeof(int)) < 0 )
			{ handle_sys_error("Setting TPACKET_V1 for this ring..."); }
	#endif
	
	// 2) open ring
  	if ( ( (*ring) = mmap(	NULL, ring_len, ring_access_flags, MAP_SHARED,
  							socket_fd, 0) ) == NULL )
	{
		log_sys_error("mmap()ing error");
		return(EX_ERR);
	}
	
	return(ring_len);
	
}

/* close_rings */
int close_rings(const ll_socket_t *ll_socket)
{

	int result = EX_OK;

	if ( munmap(ll_socket->tx_ring_buffer, ll_socket->tx_ring_len) < 0 )
	{
		log_sys_error("Closing TX ring buffer");
		result = EX_ERR;
	}

	if ( munmap(ll_socket->rx_ring_buffer, ll_socket->rx_ring_len) < 0 )
	{
		log_sys_error("Closing RX ring buffer");
		result = EX_ERR;
	}
	
	return(result);

}

#endif

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// LIBEV
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/* init_events */
int init_events(const bool is_transmitter, ll_socket_t *ll_socket)
{
	printf("ini7\n");


	if ( init_events_cb(ll_socket) < 0 )
	{
		log_app_msg("Could not properly set callback functions.\n");
		return(EX_ERR);
	}

	if ( is_transmitter == true )
	{

		printf("init_tx_events\n");
		if ( init_tx_events(ll_socket) < 0 )
			{ handle_app_error("Could not initialize TX events with libev!"); }
		log_app_msg("Frame reception is disabled.\n");

	}
	else
	{

		printf("init_rx_events\n");
		if ( init_rx_events(ll_socket) < 0 )
			{ handle_app_error("Could not initialize RX events with libev!"); }
		log_app_msg("Frame transmission is disabled.\n");

	}

	return(EX_OK);

}

/* init_events_cb */
int init_events_cb(ll_socket_t *ll_socket)
{
	printf("ini8\n");
	ev_cb_t rx_cb = NULL;
	ev_cb_t tx_cb = NULL;
	printf("go\n");
	switch(ll_socket->frame_type)
	{printf("mau\n");
		case TYPE_BUFFER:
			printf("go\n");
			log_app_msg("Buffer frame type not supported yet.\n");
			return(EX_UNSUPPORTED);

		case TYPE_IEEE_8023:

			rx_cb = (ev_cb_t)&ieee8023_frame_rx_cb;
			tx_cb = (ev_cb_t)&ieee8023_frame_tx_cb;
			break;

		case TYPE_IEEE_80211:
printf("go1\n");
			rx_cb = (ev_cb_t)&ieee80211_frame_rx_cb;
			tx_cb = (ev_cb_t)&ieee80211_frame_tx_cb; // punto clave


			break;

		default:

			log_app_msg("Unsupported frame_type = %d\n", ll_socket->frame_type);
			return(EX_UNSUPPORTED);

	}

	// Set callback functions...
	if ( set_cb_frame_rx(ll_socket, rx_cb) < 0 )
	{
		log_app_msg("Could not set frame rx callback.\n");
		return(EX_ERR);
	}
	if ( set_cb_frame_tx(ll_socket, tx_cb) < 0 )
	{
		log_app_msg("Could not set frame tx callback.\n");
		return(EX_ERR);
	}

	return(EX_OK);

}

/* init_rx_events */
int init_rx_events(ll_socket_t *ll_socket)
{
	printf("ini9\n");
	ll_socket->loop = EV_DEFAULT;
	ev_io_arg_t *arg = init_ev_io_arg(ll_socket);
	ll_socket->rx_watcher = &arg->watcher;
	printf(">2 ll_sap = %d, h_dest = ", arg->public_arg.ll_sap);
	//print_eth_address(ETH_ADDR_BROADCAST);
		//	printf(", h_source = ");
			print_eth_address(ll_socket->if_mac);
			printf("\n");
#ifdef KERNEL_RING
	ev_io_init(	ll_socket->rx_watcher, cb_process_frame_rx,
				ll_socket->rx_socket_fd,
				EV_READ	);
#else
	ev_io_init(	ll_socket->rx_watcher, cb_process_frame_rx,
				ll_socket->socket_fd,
				EV_READ	);
#endif
printf("ev_io_start\n");
	ev_io_start(ll_socket->loop, ll_socket->rx_watcher);

    return(EX_OK);

}

/* init_tx_events */
int init_tx_events(ll_socket_t *ll_socket)
{

	log_app_msg("Starting test tx in loop mode...\n");

	ll_socket->loop = EV_DEFAULT;
	ev_io_arg_t *arg = init_ev_io_arg(ll_socket);
	ll_socket->tx_watcher = &arg->watcher;

	printf(">2 ll_sap = %d, h_dest = ", arg->public_arg.ll_sap);
		print_eth_address(ETH_ADDR_BROADCAST);
		printf(", h_source = ");
		print_eth_address(arg->public_arg.if_mac);
		printf("\n");

#ifdef KERNEL_RING
	ev_io_init(	ll_socket->tx_watcher, cb_process_frame_tx,
				ll_socket->tx_socket_fd,
				EV_WRITE	);
#else
	ev_io_init(	ll_socket->tx_watcher, cb_process_frame_tx,
				ll_socket->socket_fd,
				EV_WRITE	);
#endif

	ev_io_start(ll_socket->loop, ll_socket->tx_watcher);

	return(EX_OK);

}

/* close_events */
int close_events(const ll_socket_t *ll_socket)
{

	int result = EX_OK;
	
	return(result);

}

/* cb_process_frame_rx */
void cb_process_frame_rx
	(struct ev_loop *loop, struct ev_io *watcher, int revents)
{printf("ini10\n");

	if( EV_ERROR & revents )
	{
		log_sys_error("Invalid event");
		return;
	}

	ev_io_arg_t *arg = (ev_io_arg_t *)watcher;
	public_ev_arg_t *public_arg = &arg->public_arg;
	public_arg->socket_fd = watcher->fd;

	arg->cb_frame_rx(public_arg);

}

/* cb_process_frame_tx */
void cb_process_frame_tx
	(struct ev_loop *loop, struct ev_io *watcher, int revents)
{

	if( EV_ERROR & revents )
	{
		log_sys_error("Invalid event");
		return;
	}

	ev_io_arg_t *arg = (ev_io_arg_t *)watcher;
	public_ev_arg_t *public_arg = &arg->public_arg;
	public_arg->socket_fd = watcher->fd;

	arg->cb_frame_tx(public_arg);

}

/* set_cb_frame_rx */
int set_cb_frame_rx(ll_socket_t *ll_socket, ev_cb_t cb_frame_rx)
{
	printf("ini11\n");
	if ( ll_socket == NULL )
		{ return(EX_NULL_PARAM); }

	ll_socket->cb_frame_rx = cb_frame_rx;

	return(EX_OK);

}

/* set_cb_frame_tx */
int set_cb_frame_tx(ll_socket_t *ll_socket, ev_cb_t cb_frame_tx)
{

	if ( ll_socket == NULL )
		{ return(EX_NULL_PARAM); }

	ll_socket->cb_frame_tx = cb_frame_tx;

	return(EX_OK);

}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// DATA TX/RX INTERFACE
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/* start_ll_socket */
int start_ll_socket(ll_socket_t *ll_socket)
{

	// 1) start event_loop event's reading
	log_app_msg("Starting ev_run_loop.\n");
	print_eth_address(ll_socket->if_mac);
	//printf(ll_socket->loop);
	printf("1\n");
	//printf("%d %s\n",ll_socket-> frame_type,);
	//print_eth_address(ll_socket->if_mac);
	//ll_socket->sll_protocol = htons(ETH_P_ALL);
	ev_loop(ll_socket->loop, 0);
	log_app_msg("Done ev_run_loop.\n");
	ll_socket->state = LL_SOCKET_STATE_RUNNING;

	return(EX_OK);

}
