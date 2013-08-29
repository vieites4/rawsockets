/*
 * @file configuration.h
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
 *
 * @section DESCRIPTION
 *
 * Header file where runtime configuration structures and data types are
 * defined.
 */

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include "main.h"
#include "execution_codes.h"
#include "logger.h"

#include <net/if.h>
#include <getopt.h>

/********************************************************************* EXTERN */

/*!< Variable that sets the __verbose mode on or off. */
bool __verbose;

/***************************************************************** DATA TYPES */

#define __USECS_2_MSECS 		1000	/*!< From msecs to usecs. */
#define __USECS_2_SECS			1000000	/*!< From usecs to secs. */
#define __MAX_TX_DELAY			1000	/*!< Maximum ammount of msecs. */

#define RAW_FRAME				0	/*!< RAW frame is to be read. */
#define IEEE_8023_FRAME 		1	/*!< IEEE 802.3 frame is to be read. */
#define IEEE_80211_FRAME		2	/*!< IEEE 802.11 frame is to be read. */

#define LEN__LL_IF_NAME_BUFFER ( IF_NAMESIZE + 1 )	/*!< if_name buffer size */

/*!
 * \struct configuration_t
 * \brief Runtime configuration of the application.
 */
typedef struct configuration
{
	
	bool is_transmitter;					/*!< Indicates transmission mode. */
	bool is_receiver;						/*!< Indicates reception mode. */
	
	int lsap;								/*!< LSAP to be used. */
	char if_name[LEN__LL_IF_NAME_BUFFER];	/*!< Name of the link interface. */
	
	int tx_delay;							/*!< Delay of test frames (ms).*/

	int frame_type;							/*!< Type of frame to be read. */

} configuration_t;

#define LEN__T_CONFIGURATION sizeof(configuration_t)	/*!< configuration_t */

/****************************************************************** FUNCTIONS */

/*!
 * \brief Creates an structure with the configuration read from the console.
 * \param argc Number of parameters given by the cli.
 * \param argv Array with the parameters from the cli.
 * \return A pointer to the structure that contains the configuration read.
 */
configuration_t *create_configuration(int argc, char** argv);

/*!
 * \brief Allocates memory for the a configuration structure.
 * \return A pointer to the newly allocated block of memory.
 */
configuration_t *new_configuration();

/*!
 * \brief Reads the configuration from the command line arguments.
 * \param argc Number of parameters given by the cli.
 * \param argv Array with the parameters from the cli.
 * \param cfg Structure where the configuration is to be stored.
 * \return EX_OK in case the function was correctly executed.
 */
int read_configuration(int argc, char** argv, configuration_t* cfg);

/*!
 * \brief Checks the correctness of the configuration read.
 * \param cfg Structure where the configuration is stored.
 * \return EX_OK in case the function was correctly executed.
 */
int check_configuration(configuration_t *cfg);

/*!
 * \brief Prints the values configuration structure.
 * \param cfg Structure where the configuration is stored.
 */
void print_configuration(const configuration_t *cfg);

#endif /* CONFIGURATION_H_ */
