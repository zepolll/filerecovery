#include "mbr.h"
#include "vbr.h"
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>


#define DEBUG 1

bool vbrVerify( struct disk_ntfs_vbr *buf ) {

#ifdef DEBUG
    printf("vbrVerify: vbrObj.oem_id = %x, %x, %x, %x, %x, %x, %x, %x\n",
          buf->oem_id[0],
          buf->oem_id[1],
          buf->oem_id[2],
          buf->oem_id[3],
          buf->oem_id[4],
          buf->oem_id[5],
          buf->oem_id[6],
          buf->oem_id[7] );
#endif

   if ( (buf->oem_id[0] == 'N') && (buf->oem_id[1] == 'T') && (buf->oem_id[2] == 'F') && (buf->oem_id[3] == 'S') ) {
      return (true);
   }
   else {
      return (false);
   }
}

bool verifyMbr( unsigned char *buf ) {

   if ( (buf[0x1fe] != 0x55) && (buf[0x1ff] != 0xaa) )
   {
      fprintf(stderr, "verifyMbr: MBR corrupted ... \n");
      return(false);
   }

   return(true);

}

unsigned long long getVbrAddr( DISK_mbr *mbr ) {

   return( mbr->pt[0].first_sector_lba  * 512 );

}

unsigned long long getMftAdd( int fd ) {
   bool ret;
   DISK_mbr mbrBuf;
   struct disk_ntfs_vbr vbrBuf;
   unsigned char *buf;
   int retVal;
   unsigned long long vbrAddr;
   unsigned long long mftAddr;
   unsigned long long curLoc;
   unsigned short bytesPerCluster;

#ifdef DEBUG
   fprintf(stderr, "enter getMftAdd\n");
#endif

   buf = (unsigned char *)&mbrBuf;

   retVal = read( fd, buf, sizeof(DISK_mbr) ); 
   if ( retVal < 0 ) {
      fprintf(stderr, "getMftAdd: unable to read disk, retVal = %d\n", retVal );
   }

/* Verify the magic values for MBR */
#ifdef DEBUG
   printf( "getMftAdd: Magicval1 = 0x%x  magicval2 = 0x%x\n",
            buf[0x1fe], buf[0x1ff] );
#endif
   ret = verifyMbr( buf );
   if ( ret == false ) {
      return( 0 );
   }

   vbrAddr = getVbrAddr( &mbrBuf );

   if ( vbrAddr == 0 ) {
      fprintf(stderr, "getMftAdd: VBR address 0\n");
      return( 0 );
   }

   curLoc = lseek( fd, vbrAddr, SEEK_SET );
   if ( curLoc <=0 ) {
      fprintf( stderr, "getMftAdd: unable to lseek \n" );
      return ( 0 );
   }

   buf = (unsigned char *)&vbrBuf;

   retVal = read(fd, buf, sizeof(vbrBuf) );
   if ( retVal < 0 ) {
      fprintf(stderr, "getMftAddunable to read disk, retVal = %d\n", retVal );
      return( 0 );
   }

   ret = vbrVerify( &vbrBuf );
   if ( ret == false ) {
      fprintf( stderr, "getMftAdd: vbr verify failed\n" );
      return( 0 );
   }

   bytesPerCluster = vbrBuf.bpb.sectorsPerCluster * 512;

   mftAddr = vbrBuf.bpb.clusterNumForMFT * bytesPerCluster + vbrAddr;

   printf( "%llx\n", vbrAddr );
   printf( "%llx\n", mftAddr );

   return( mftAddr );

}
