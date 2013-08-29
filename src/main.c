/*
 * @file main.c
 * @author Ricardo Tubío (rtpardavila[at]gmail.com)
 * @version 0.1
 *
 * @section LICENSE
 *
 *  Created on: May 6, 2013
 *      Author: Ricardo Tubío (rtpardavila[at]gmail.com)
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

#include "main.h"

#include "logger.h"
#include "configuration.h"
#include "ll_library/ll_socket.h"
#include "ll_library/ieee8023_frame.h"

/**************************************************** Application definitions */
static const char* __x_app_name = "linklayertool";
static const char* __x_app_version = "0.1";

/********************************************************* INTERNAL FUNCTIONS */

/* print_help */
void print_help()
{
	fprintf(stdout, "HELP, %s\n", __x_app_name);
}

/* print_version */
void print_version()
{
	fprintf(stdout, "Version = %s\n", __x_app_version);
}

/* transmit_data */
int transmit_data(ll_socket_t *ll_socket)
{
	return(EX_OK);
}

/* receive_data */
int receive_data(ll_socket_t *ll_socket)
{
	return(EX_OK);
}

/* main */
int main(int argc, char **argv)
{

	configuration_t *cfg = NULL;
	ll_socket_t *ll_socket = NULL;
	
	/* 1) Runtime configuration is read from the CLI (POSIX.2). */
	cfg = create_configuration(argc, argv);
	print_configuration(cfg);

	/* 2) Link layer socket is open. */
	if ( ( ll_socket = open_ll_socket
						(	cfg->is_transmitter,
							cfg->tx_delay,
							cfg->if_name,
							cfg->lsap,
							cfg->frame_type	)
						) == NULL )
		{ handle_app_error("Could not open ll_socket.\n"); }
	printf("descriptor %d\n",ll_socket->socket_fd);

	#ifdef KERNEL_RING
		log_app_msg("TX socket open with fd = %d\n", ll_socket->tx_socket_fd);
		log_app_msg("RX socket open with fd = %d\n", ll_socket->rx_socket_fd);
	#else
		log_app_msg("Socket open with fd = %d\n", ll_socket->socket_fd);
	#endif

	/* 3) Set-up this programe either as a transmitter or a receiver. */
	if ( cfg->is_transmitter == true )
	{
		log_app_msg("Setting up transmitter mode...\n");
	}
	else
	{
		log_app_msg("Setting up receiver mode...\n");
	}

	//exit(EXIT_SUCCESS);

	start_ll_socket(ll_socket);

	// 4) sockets are closed before exiting application
	close_ll_socket(ll_socket);
	log_app_msg("Socket is closed.\n");

	exit(EXIT_SUCCESS);

}
