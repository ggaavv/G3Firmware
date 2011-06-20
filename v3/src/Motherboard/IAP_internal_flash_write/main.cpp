/**    IAP demo : demo code for internal Flash memory access library
 *
 *        The internal Flash memory access is described in the LPC1768 usermanual. 
 *            http://www.nxp.com/documents/user_manual/UM10360.pdf
 *
 *            Chapter  2: "LPC17xx Memory map"
 *            Chapter 32: "LPC17xx Flash memory interface and programming"
 *                refering Rev. 01 - 4 January 2010
 *
 *  This main.cpp demonstrates how the flash can be erased and wrote.
 *
 *  This program tries to...
 *    0. read device ID and serial#
 *    1. check if the targat sector blank
 *    2. erase the sector if it was not blank
 *    3. write into the flash (prepare before write)
 *    4. verify the data by IAP command
 *    5. show the content of the flash
 *
 *  The Flash must be erased as sectors. No overwrite can be done like SRAM. 
 *  So erase should be done in size of 4K or 32K. 
 *
 *  Writing sector can be done with size of 256, 512, 1024 or 4096. 
 *  If other size is used, the IAP returns an error. 
 *  The SRAM memory should be allocated in 
 *
 *
 *  Released under the MIT License: http://mbed.org/license/mit
 *
 *        revision 1.0  09-Mar-2010   1st release
 *        revision 1.1  12-Mar-2010   chaged: to make possible to reserve flash area for user
 *                                            it can be set by USER_FLASH_AREA_START and USER_FLASH_AREA_SIZE in IAP.h
 */

#include    "mbed.h"
#include    "IAP.h"

#define     MEM_SIZE        256
#define     TARGET_SECTOR    29

void    memdump( char *p, int n );
int     isprint( int c );

IAP     iap;


int main() {
    char    mem[ MEM_SIZE ];    //  memory, it should be aligned to word boundary
    int     r;

    printf( "IAP: Flash memory writing test\n" );
    printf( "  device-ID = 0x%08X, serial# = 0x%08X, CPU running %dkHz\n", iap.read_ID(), iap.read_serial(), SystemCoreClock / 1000 );
    printf( "  user reserved flash area: start_address=0x%08X, size=%d bytes\n", iap.reserved_flash_area_start(), iap.reserved_flash_area_size() );

    for ( int i = 0; i < MEM_SIZE; i++ )
        mem[ i ]    = i & 0xFF;

    //  blank check: The mbed will erase all flash contents after downloading new executable

    r   = iap.blank_check( TARGET_SECTOR, TARGET_SECTOR );
    printf( "blank check result = 0x%08X\n", r );

    //  erase sector, if required
    
    if ( r == SECTOR_NOT_BLANK ) {
        iap.prepare( TARGET_SECTOR, TARGET_SECTOR );
        r   = iap.erase( TARGET_SECTOR, TARGET_SECTOR );
        printf( "erase result       = 0x%08X\n", r );
    }
    
    // copy RAM to Flash

    iap.prepare( TARGET_SECTOR, TARGET_SECTOR );
    r   = iap.write( mem, sector_start_adress[ TARGET_SECTOR ], MEM_SIZE );
    printf( "copied: SRAM(0x%08X)->Flash(0x%08X) for %d bytes. (result=0x%08X)\n", mem, sector_start_adress[ TARGET_SECTOR ], MEM_SIZE, r );

    // compare

    r   = iap.compare( mem, sector_start_adress[ TARGET_SECTOR ], MEM_SIZE );
    printf( "compare result     = \"%s\"\n", r ? "FAILED" : "OK" );

//#define WRITE_NEXT_BLOCK
#ifdef WRITE_NEXT_BLOCK

    // copy RAM to Flash

    iap.prepare( TARGET_SECTOR, TARGET_SECTOR );
    r   = iap.write( mem, sector_start_adress[ TARGET_SECTOR ] + 256, MEM_SIZE );
    printf( "copied: SRAM(0x%08X)->Flash(0x%08X) for %d bytes. (result=0x%08X)\n", mem, sector_start_adress[ TARGET_SECTOR ], MEM_SIZE, r );

    // compare

    r   = iap.compare( mem, sector_start_adress[ TARGET_SECTOR ] + 256, MEM_SIZE );
    printf( "compare result     = \"%s\"\n", r ? "FAILED" : "OK" );

#endif

    printf( "showing the flash contents...\n" );
    memdump( sector_start_adress[ TARGET_SECTOR ], MEM_SIZE * 3 );
}


void memdump( char *base, int n ) {
    unsigned int    *p;

    printf( "  memdump from 0x%08X for %d bytes", (unsigned long)base, n );

    p   = (unsigned int *)((unsigned int)base & ~(unsigned int)0x3);

    for ( int i = 0; i < (n >> 2); i++, p++ ) {
        if ( !(i % 4) )
            printf( "\n  0x%08X :", (unsigned int)p );

        printf( " 0x%08X", *p );
    }

    printf( "\n" );
}
