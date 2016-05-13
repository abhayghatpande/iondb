/*
 * kv_io.h
 *
 *  Created on: May 23, 2014
 *      Author: workstation
 */

#ifndef KV_IO_H_
#define KV_IO_H_

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "kv_system.h"

#ifndef ARDUINO /* Only on PC */
#define fremove(x) remove(x)
#define frewind(x) rewind(x)
#endif

/**
 * @brief Allows for output in a tidy format.
 *
 * @details Used as an alternate printf to deal with device specific issues
 *
 * @param format
 * @return
 */
int io_printf( const char * format, ... );

#endif /* KV_IO_H_ */
