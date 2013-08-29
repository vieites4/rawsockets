/*
 * @file logger.h
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
 * Header file with the definitions for a very simple logging utility.
 *
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <stdio.h>
#include <stdlib.h>

#define handle_sys_error(msg) \
           	do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define handle_fd_error(fd, msg, ex_no) \
           	do { perror(msg); close(fd); return(ex_no); } while (0)
           	
#define log_sys_error(msg) \
           	do { perror(msg); fprintf(stderr, "err# = %d\n", errno); } while (0)
           	
#define handle_app_error(...) \
           	do { fprintf(stderr, __VA_ARGS__); exit(EXIT_FAILURE); } while (0)

#define log_app_msg(...) \
			do { fprintf(stdout, __VA_ARGS__); } while(0)

#endif /* LOGGER_H_ */

