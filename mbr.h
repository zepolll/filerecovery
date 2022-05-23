#ifndef __MBR__

#define __MBR__
#include <stdint.h>

typedef struct mbr_partition_table_entry
{
    uint8_t status;
    uint8_t start_chs[3];
    uint8_t partition_type;
    uint8_t end_chs[3];
    uint32_t first_sector_lba;
    uint32_t sector_count;
} MBR_partition_table_entry;


typedef struct disk_mbr
{
    uint8_t code[440];
    uint32_t disk_signature;
    uint16_t reserved;
    MBR_partition_table_entry pt[4];
    uint8_t signature[2];
} __attribute__((packed)) DISK_mbr;

#endif
