#ifndef __VBR__

#define __VBR__
#include <linux/types.h>

extern __u16 BytesPerSector;

extern __u16 BytesPerCluster;

extern __u64 PartitionSizeClusters;


extern __u64 MftClusterNum;

extern __u64 MftMirrorClusternum;

struct vbr_ntfs_bpb
{
    __u16 bytesPerSector;
    __u8  sectorsPerCluster;
    __u16 reserved;
    __u8  unused[3];
    __u16 unused2;
    __u8  mediaType;
    __u16 unused3;
    __u16 sectorsPerTrack;
    __u16 numberOfHeads;
    __u32 hiddenSectors;
    __u32 unused4;
    __u32 unused5;
    __u64 partitionSizeInSectors;
    __u64 clusterNumForMFT;
    __u64 clusterNumForMirrorMFT;
    __u8  fileRecordSize;
    __u8  unused6[3];
    __u8  indexBufferSize;
    __u8  unused7[3];
    __u64 volumeSerNum;
    __u32 chksum;
}__attribute__((packed));

struct disk_ntfs_vbr
{
    __u8 jmp_ins[3];
    __u8 oem_id[8];
    struct vbr_ntfs_bpb bpb;
    __u8 code[426];
    __u8 signature[2];
}__attribute__((packed));

#endif

