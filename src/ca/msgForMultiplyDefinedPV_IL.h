
/*  
 *  $Id$
 *
 *                              
 *                    L O S  A L A M O S
 *              Los Alamos National Laboratory
 *               Los Alamos, New Mexico 87545
 *                                  
 *  Copyright, 1986, The Regents of the University of California.
 *                                  
 *           
 *	Author Jeffrey O. Hill
 *	johill@lanl.gov
 *	505 665 1831
 */

void * msgForMultiplyDefinedPV::operator new ( size_t size )
{
    return msgForMultiplyDefinedPV::freeList.allocate ( size );
}

void msgForMultiplyDefinedPV::operator delete ( void *pCadaver, size_t size )
{
    msgForMultiplyDefinedPV::freeList.release ( pCadaver, size );
}
