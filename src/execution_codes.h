/*
 * @file execution_codes.h
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
 * Header file with the definitions of the return values to be used when an 
 * integer is required for indicating which was the result of the execution 
 * of a given function.
 * 
 * These codes are defined following the convention that an error is always a
 * negative number, but the EX_OK result, which is zero.
 */

#ifndef EXECUTION_CODES_H_
#define EXECUTION_CODES_H_

#define EX_OK			0		/*!< Execution correct. */
#define EX_ERR			-1		/*!< Generic execution error. */
#define EX_EOF			-2		/*!< End-of-file indicator. */
#define EX_SYS			-3		/*!< System error. */
#define EX_UNSUPPORTED	-4		/*!< Operation not supported. */

#define EX_EMPTY_PARAM	-30		/*!< Empty input parameter. */
#define EX_NULL_PARAM	-31		/*!< NULL input parameter. */
#define EX_WRONG_PARAM	-32		/*!< Wrong input parameter. */

#endif /* EXECUTION_CODES_H_ */

