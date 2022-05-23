#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>

#include <stdio.h>
 
//   #define DEBUG 1


extern unsigned long long getMftAdd( int fd );

int main(int argc, char** argv) {
        int fd;
        short arg2Len;
	unsigned long long mftaddr;
        bool ret;

        arg2Len = strlen(argv[1]);
#ifdef DEBUG
        printf("main: arglen =  %d\n", arg2Len );
#endif

        if ( arg2Len != 8 ) {
           fprintf(stderr, "USAGE: %s /dev/sdx #\n", argv[0]);
           return(0);
        }

        if(argc != 4) {
           fprintf(stderr, "(0)USAGE: %s /dev/sdx\n", argv[0]);
           return(0);
        }

        fd = open(argv[1], O_RDONLY);
        if ( fd < 0 )
        {
                fprintf(stderr, "device not opened = %s \n", argv[1] );
                return(0);
        }
        mftaddr = getMftAdd( fd );
	//printf("mftaddr = %llx\n", mftaddr );
	//travEntry() call passing in mft address
	//travEntry(fd, mftaddr);
	//entry number
	int entry = atoi(argv[2]);
	//fprintf(stdout, "entry #: %d\n", entry);
	nameInfo(fd, mftaddr, entry,argv[3]);
        close( fd );
}

