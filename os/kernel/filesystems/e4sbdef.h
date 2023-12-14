#ifndef __E4SBDEF_H__
#define __E4SBDEF_H__ 1

#define E4SB_INODES_COUNT 0
#define E4SB_BLOCKS_COUNT_LO 4
#define E4SB_R_BLOCKS_COUNT_LO 8
#define E4SB_R_BLOCKS_COUNT_LO 0xc
#define E4SB_FREE_BLOCKS_COUNT_LO 0x10
#define E4SB_FREE_INODES_COUNT 0x14
#define E4SB_FIRST_DATA_BLOCK 0x18
#define E4SB_LOGICAL_BLOCK_SIZE 0x1c
#define E4SB_BLOCKS_PER_GROUP 0x20
#define E4SB_CLUSTERS_PER_GROUP 0x24
#define E4SB_INODES_PER_GROUP 0x28
#define E4SB_MTIME 0x2c
#define E4SB_WTIME 0x30
#define E4SB_MOUNT_COUNT 0x34
#define E4SB_MAX_MOUNT_COUNT 0x36
#define E4SB_MAGIC 0x38
#define E4SB_STATE 0x3a
#define E4SB_ERRORS 0x3c
#define E4SB_MINOR_REVISION_LEVEL 0x3e
#define E4SB_LAST_CHECK 0x40
#define E4SB_CHECK_INTERVAL 0x44
#define E4SB_CREATOR_OS 0x48
#define E4SB_REVISION_LEVEL 0x4c
#define E4SB_DEF_RESUID 0x50
#define E4SB_DEF_RESGID 0x52
#define E4SB_FIRST_INODE 0x54
#define E4SB_INODE_SIZE 0x58
#define E4SB_BLOCK_GROUP_NUMBER 0x5a
#define E4SB_FEATURE_COMPAT 0x5c
#define E4SB_FEATURE_INCOMPAT 0x60
#define E4SB_RO_FEATURE_COMPAT 0x64
#define E4SB_UUID 0x68 // 128-bit uuid
#define E4SB_VOLUME_NAME 0x78 // 16-byte volume name
#define E4SB_LAST_MOUNTED 0x88 // 64-byte directory name
#define E4SB_ALGORITHM_USAGE_BITMAP 0xc8
#define E4SB_PREALLOC_BLOCKS 0xcc
#define E4SB_PREALLOC_DIR_BLOCKS 0xcd
#define E4SB_RESERVED_GDT_BLOCKS 0xce
#define E4SB_JOURNAL_UUID 0xd0 // 128-bit uuid of journal superblock
#define E4SB_JOURNAL_INUM 0xe0 // inode number of journal file
#define E4SB_JOURNAL_DEV 0xe4 // Device number of journal file, if the external journal feature flag is set.
#define E4SB_LAST_ORPHAN 0xe8 // start of list of orphaned inodes to delete
#define E4SB_HASH_SEED 0xec // 4x 32-bit HTREE hash seed
#define E4SB_DEFAULT_HASH_VERSION 0xfc // default hash algorithm for directory hashes
#define E4SB_JNL_BACKUP_TYPE 0xfd
#define E4SB_GROUP_DESCRIPTOR_SIZE 0xfe // size in bytes if 64-bit incompat flag is set
#define E4SB_DEFAULT_MOUNT_OPTS 0x100
#define E4SB_FIRST_META_BLOCK_GROUP 0x104 // if meta_bg flag feature is enabled
#define E4SB_MKFS_TIME 0x108 // when fs was made, in seconds since the epoch
#define E4SB_JNL_BLOCKS 0x10c // Backup copy of the journal inode's i_block[] array in the first 15 elements and i_size_high and i_size in the 16th and 17th elements, respectively.
#define E4SB_BLOCKS_COUNT_HI 0x150
#define E4SB_R_BLOCKS_COUNT_HI 0x154
#define E4SB_FREE_BLOCKS_COUNT_HI 0x158
#define E4SB_MIN_EXTRA_ISIZE 0x15c // all inodes have at least # bytes
#define E4SB_WANT_EXTRA_ISIZE 0x15e // new inodes should reserve # bytes
#define E4SB_FLAGS 0x160
#define E4SB_RAID_STRIDE 0x164
#define E4SB_MMP_INTERVAL 0x166 // apparently unused ?
#define E4SB_MMP_BLOCK 0x168
#define E4SB_RAIDE_STRIPE_WIDTH 0x170
#define E4SB_LOGICAL_GROUPS_PER_FLEX 0x174
#define E4SB_CHECKSUM_TYPE 0x175 // Metadata checksum algorithm type. The only valid value is 1 (crc32c).
#define E4SB_RESERVED_PAD 0x176
#define E4SB_KBYTES_WRITTEN 0x178 // Number of KiB written to this filesystem over its lifetime.
#define E4SB_SNAPSHOT_INUM 0x180
#define E4SB_SNAPSHOT_ID 0x184
#define E4SB_SNAPSHOT_R_BLOCKS_COUNT 0x188
#define E4SB_SNAPSHOT_LIST 0x190
#define E4SB_ERROR_COUNT 0x194
#define E4SB_ERROR_TIME 0x198 // First time an error happened, in seconds since the epoch.
#define E4SB_FIRST_ERROR_INODE 0x19c
#define E4SB_FIRST_ERROR_BLOCK 1a0
#define E4SB_FIRST_ERR_FUNC 0x1a8 // 32x u8, Name of function where the error happened.
#define E4SB_FIRST_ERROR_LINE 0x1c8
#define E4SB_LAST_ERROR_TIME 0x1cc
#define E4SB_LAST_ERROR_INODE 0x1d0
#define E4SB_LAST_ERROR_BLOCK 0x1d8
#define E4SB_LAST_ERR_FUNC 0x1e0
#define E4SB_MOUNT_OPTS 0x200 // ASCIIZ string of mount options.
#define E4SB_USER_QUOTA_INODE_NUMBER 0x240
#define E4SB_GROUP_QUOTA_INODE_NUMBER 0x244
#define E4SB_OVERHEAD_BLOCKS 0x248
#define E4SB_BACKUP_BGS 0x24c // 2x u32
#define E4SB_ENCRYPT_ALGOS 0x254 // 4x u8, Encryption algorithms in use. There can be up to four algorithms in use at any time
#define E4SB_ENCRYPT_PW_SALT 0x258 // 16x u8, Salt for the string2key algorithm for encryption.
#define E4SB_LOST_AND_FOUND_INODE 0x268
#define E4SB_PROJECT_QUOTA_INODE 0x26c
#define E4SB_CHECKSUM_SEED 0x270 // Checksum seed used for metadata_csum calculations. This value is crc32c(~0, $orig_fs_uuid).
#define E4SB_WTIME_HI 0x274
#define E4SB_MTIME_HI 0x275
#define E4SB_MKFS_TIME_HI 0x276
#define E4SB_LASTCHECK_HI 0x277
#define E4SB_FIRST_ERROR_TIME_HI 0x278
#define E4SB_LAST_ERROR_TIME_HI 0x279
#define E4SB_PAD 0x27a // 2x u8, zero padding
#define E4SB_ENCODING 0x27c // filename charset encoding
#define E4SB_ENCODING_FLAGS 0x27e
#define E4SB_ORPHAN_FILE_INUM 0x280
#define E4SB_RESERBED 0x284 // padding to end of block
#define E4SB_CHECKSUM 0x3fc // superblock checksum
#endif