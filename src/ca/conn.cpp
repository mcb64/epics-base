/* $Id$ */
/*
 *
 *                    L O S  A L A M O S
 *              Los Alamos National Laboratory
 *               Los Alamos, New Mexico 87545
 *
 *  Copyright, 1986, The Regents of the University of California.
 *
 *  Author: Jeff Hill
 */

#include "iocinf.h"

#ifdef DEBUG
#define LOGRETRYINTERVAL logRetryInterval(__FILE__, __LINE__);
LOCAL void logRetryInterval (pcac, char *pFN, unsigned lineno);
#else
#define LOGRETRYINTERVAL 
#endif

