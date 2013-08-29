/*
 * @file configuration.c
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

#include "configuration.h"

/* new_configuration */
configuration_t *new_configuration()
{

	configuration_t *cfg = NULL;
	cfg = (configuration_t *)malloc(LEN__T_CONFIGURATION);
	return(cfg);

}

/* create_configuration */
configuration_t *create_configuration(int argc, char** argv)
{

	configuration_t *cfg = new_configuration();
	
	read_configuration(argc, argv, cfg);
	check_configuration(cfg);
	
	return(cfg);
	
}

/* read_configuration */
int read_configuration(int argc, char** argv, configuration_t* cfg)
{

	int index = 0, read = 0;
    
    static struct option args[] =
	{
		{"help",	no_argument,		NULL, 	'h'	},
		{"verbose",	no_argument,		NULL,	'e'	},
		{"version",	no_argument,		NULL, 	'v'	},
		{"tx",		required_argument,	NULL, 	't'	},
		{"rx",  	no_argument,		NULL, 	'r'	},
		{"lsap", 	required_argument,	NULL, 	'l'	},
		{"if",		required_argument,	NULL,	'i'	},
		{"frame", 	required_argument, 	NULL,	'f'	},
		{0,0,0,0}
	};
	
	while
		( ( read = getopt_long(argc, argv, "ehvt:rl:i:f:", args, &index) )
				> -1 )
	{
		
		switch(read)
		{
			case 't':
				
				cfg->is_transmitter = true;
				cfg->tx_delay = atoi(optarg);
				break;
				
			case 'r':
				
				cfg->is_receiver = true;
				break;
				
			case 'l':
			
				cfg->lsap = atoi(optarg);
				break;
			
			case 'i':
			
				if ( strlen(optarg) > IF_NAMESIZE )
				{
					log_app_msg("[WARNING] if_name length = %d, maximum = %d.\
								TRUNCATING!\n", (int)strlen(optarg), \
								IF_NAMESIZE);
				}
				
				strncpy(cfg->if_name, optarg, IF_NAMESIZE);
				break;

			case 'f':

				cfg->frame_type = atoi(optarg);
				log_app_msg("cfg->frame_type = %d\n", cfg->frame_type);
				break;

			case 'e':
				
				__verbose = true;
				break;

			case 'v':
			
				print_version();
				exit(EXIT_SUCCESS);
				break;
				
			case 'h':
			default:
			
				print_help();
				exit(EXIT_SUCCESS);
				break;
		}

	};

	return(EX_OK);

}

/* check_configuration */
int check_configuration(configuration_t *cfg)
{

	if ( cfg == NULL )
	{
		handle_app_error("Configuration to be checked is NULL\n");
	}

	if ( ! ( cfg->is_transmitter != cfg->is_receiver ) )
	{
		handle_app_error("Only TX or RX can be activated at a time.\n");
	}

	if ( cfg->is_transmitter == true )
	{
		if ( ( cfg->tx_delay <= 0 ) || ( cfg->tx_delay > __MAX_TX_DELAY ) )
		{
			handle_app_error
				("tx_delay must be bigger than 0 and smaller than %d.\n"
						, __MAX_TX_DELAY);
		}

		cfg->tx_delay = __USECS_2_MSECS * cfg->tx_delay;

	}

	if ( cfg->lsap <= 0 )
	{
		handle_app_error("Wrong LSAP = %d, shall be bigger than zero.\n"
						, cfg->lsap);
	}
	
	if ( strlen(cfg->if_name) <= 0  )
	{
		handle_app_error("Link Layer interface name must be provided.\n");
	}

	if ( cfg->frame_type <= 0  )
	{
		log_app_msg("A single type of frame must be selected:\n");
		log_app_msg("\t* IEEE 802.3 frame = %d\n", IEEE_8023_FRAME);
		handle_app_error("\t* IEEE 802.11 frame = %d\n", IEEE_80211_FRAME);
	}

	return(EX_OK);

}

/* print_configuration */
void print_configuration(const configuration_t *cfg)
{

	if ( cfg == NULL )
	{
		handle_app_error("Given configuration is NULL.\n");
	}
	
	log_app_msg(">>> Configuration = \n{\n");
	log_app_msg("\t.is_transmitter = %d\n", cfg->is_transmitter);
	log_app_msg("\t.is_receiver = %d\n", cfg->is_receiver);
	log_app_msg("\t.lsap = %d\n", cfg->lsap);
	log_app_msg("\t.if_name = %s\n", cfg->if_name);
	log_app_msg("}\n");
	
}

