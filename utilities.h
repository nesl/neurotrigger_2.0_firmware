/*
  #################################################################################
  # COLLECTION USEFUL UTILITIES
  # ------------------------------------------------------------------------------
  # A set of common macros and utilities that are useful
  # 
  # APPLIES TO (Ragobot Part Numbers):
  # ------------------------------------------------------------------------------
  # ALL
  #
  # COPYRIGHT NOTICE
  # ------------------------------------------------------------------------------
  # "Copyright (c) 2000-2003 The Regents of the University  of California.
  # All rights reserved.
  # Permission to use, copy, modify, and distribute this software and its
  # documentation for any purpose, without fee, and without written agreement is
  # hereby granted, provided that the above copyright notice, the following
  # two paragraphs and the author appear in all copies of this software.
  # IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
  # DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
  # OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
  # CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  # THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
  # INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
  # AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
  # ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
  # PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
  #
  # DEVELOPED BY
  # ------------------------------------------------------------------------------
  # Jonathan Friedman, GSR
  # Networked and Embedded Systems Laboratory (NESL)
  # University of California, Los Angeles (UCLA)
  #
  # REVISION HISTORY
  # ------------------------------------------------------------------------------
  # Original by Jonathan Friedman, UCLA (jf@ee.ucla.edu) 		
  #################################################################################
*/
#ifndef __utilities_h__
#define __utilities_h__
	
typedef uint8_t boolean;	
#define true 	1
#define false 	0
#define ENABLE	42
#define DISABLE 41
	
/* Binary constant generator macro
   By Tom Torfs - donated to the public domain

   Sample usage:
   B8(01010101) = 85
   B16(10101010,01010101) = 43605
   B32(10000000,11111111,10101010,01010101) = 2164238933
*/

/* All macro's evaluate to compile-time constants */

/* *** helper macros *** */

/* turn a numeric literal into a hex constant
   (avoids problems with leading zeroes)
   8-bit constants max value 0x11111111, always fits in unsigned long
*/
#define HEX__(n) 0x##n##LU

/* 8-bit conversion function */
#define B8__(x) ((x&0x0000000FLU)?1:0)	\
               +((x&0x000000F0LU)?2:0)	\
               +((x&0x00000F00LU)?4:0)	\
               +((x&0x0000F000LU)?8:0)	\
               +((x&0x000F0000LU)?16:0)	\
               +((x&0x00F00000LU)?32:0)	\
               +((x&0x0F000000LU)?64:0)	\
               +((x&0xF0000000LU)?128:0)

/* *** user macros *** */

/* for upto 8-bit binary constants */
#define B8(d) ((unsigned char)B8__(HEX__(d)))

/* for upto 16-bit binary constants, MSB first */
#define B16(dmsb,dlsb) (((unsigned short)B8(dmsb)<<8)	\
			+ B8(dlsb))

/* for upto 32-bit binary constants, MSB first */
#define B32(dmsb,db2,db3,dlsb) (((unsigned long)B8(dmsb)<<24)	 \
				  + ((unsigned long)B8(db2)<<16) \
				  + ((unsigned long)B8(db3)<<8)	 \
				  + B8(dlsb))

#endif
