//
///  myfs-structs.h
///  myfs
//
///  Created by Oliver Waldhorst on 07.09.17.
///  Copyright © 2017 Oliver Waldhorst. All rights reserved.
///

#include <sys/stat.h>
#include <cstdio>
#include <ctime>
#include <unistd.h>

#ifndef myfs_structs_h
#define myfs_structs_h

#define NAME_LENGTH 255
#define BLOCK_SIZE 512
#define NUM_DIR_ENTRIES 64
#define NUM_OPEN_FILES 64
#define NUM_DATA_BLOCKS 2097152 ///Block count of dataBlocks
#define NUM_FETA_BLOCKS 65536 ///Block count of Fat


#define NUM_BLOCK_SUM 2500000
#define EMPTY_BLOCK 0x00000000
#define EOF_BLOCK 0xFFFFFFFF


/*!
 * @brief Base Struct of all Files
 *
 * @param name name of the file
 * @param size size of the file
 * @param mode permission level
 * @param user_id user id
 * @param group_id group id
 * @param atime time off last access
 * @param mtime time off last modification
 * @param ctime time off last status change
 * @param data content of the file
 */

struct MyFsFile {
public:
    char name[NAME_LENGTH];
    size_t size;
    mode_t mode;
    uid_t user_id;
    gid_t group_id;
    nlink_t link = 1;
    time_t atime;
    time_t mtime;
    time_t ctime;
};


/*!
 *  @brief Extending Base File Struct for In-Memory use!
 *  @param data: Pointer to data of File
 */
struct MyFsFileInMemory : MyFsFile {

    // Achtung: Hat keinen Nullterminator!
    char* data;
};

/*!
 *  @brief Extending Base File Struct for On-Memory use!
 *  @param firstBlock: Reference of Starting Block
 */
struct MyFsFileOnMemory : MyFsFile {

    int firstBlock = -1;
};

/*!
 * @brief MyFsSuperBlock defines starting index of FAT, ROOT and DATA
 * @param FAT: Start of Fat
 * @param ROOT: Start of Root
 * @param DATA: Start of Data
 */
struct MyFsSuperBlock {
    int32_t FAT = (sizeof(MyFsSuperBlock)/BLOCK_SIZE)+1;
    int32_t root = FAT + (32*2500000/512);
    int32_t data = root + (sizeof(MyFsFileOnMemory)*NUM_DIR_ENTRIES/512);
};

/* Structure: |SuperBlock|FAT|ROOT|DATA|
 * FAT organizes all Blocks => blockSize(SuperBlock) + blockSize(FAT) + blockSize(ROOT) + blockSize(DATA)
 * FAT can store DMAP Data with this type of implementation
 * Empty Blocks get 0x00000000; EOF Blocks get 0xFFFFFFFF
 * Root Block Size: (sizeof MyFsFileOnMemory * 64)/512 = 320*64/512 = 40
 * => 2.500.000 Blocks ca. 1,20 GiB in Sum => FAT = 156.250
 *                                  SP = 1
 *                                  Root = 40
 *                                  => DATA = 2.500.000 - 1 - 40 = 2.343.709 ca. 1,12 GiB
 */

#endif /* myfs_structs_h */
