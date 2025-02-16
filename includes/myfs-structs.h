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
#include <cmath>

#ifndef myfs_structs_h
#define myfs_structs_h

#define BLOCK_SIZE 512
#define SIZE_MiB 100
#define SIZE_BYTE (SIZE_MiB<<20)

#define NUM_DATA_BLOCKS ((int) (SIZE_BYTE / BLOCK_SIZE))

#define NAME_LENGTH 255

#define NUM_DIR_ENTRIES 64

#define EMPTY_BLOCK (-2)
#define EOC_BLOCK (-1)


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
    const int32_t FAT = (sizeof(MyFsSuperBlock)/BLOCK_SIZE)+1;
    int32_t root = FAT + (NUM_DATA_BLOCKS*sizeof(int)/BLOCK_SIZE);
    int32_t data = root + (sizeof(MyFsFileOnMemory)*NUM_DIR_ENTRIES/BLOCK_SIZE);
};

namespace MyFs_Namespace {
    extern int openFiles;
    extern int FAT[NUM_DATA_BLOCKS];
    extern MyFsFileOnMemory root[NUM_DIR_ENTRIES];
    extern MyFsSuperBlock myFsSuperBlock;
}

#endif /* myfs_structs_h */
