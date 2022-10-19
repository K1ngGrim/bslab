//
///  myfs-structs.h
///  myfs
//
///  Created by Oliver Waldhorst on 07.09.17.
///  Copyright © 2017 Oliver Waldhorst. All rights reserved.
///

#include <stat.h>

#ifndef myfs_structs_h
#define myfs_structs_h

#define NAME_LENGTH 255
#define BLOCK_SIZE 512
#define NUM_DIR_ENTRIES 64
#define NUM_OPEN_FILES 64

// TODO: Add structures of your file system here

/*!
 * @brief file
 *
 * @param name name of the file
 * @param size size of the file
 * @param mode permission level
 * @param user_id user id
 * @param group_id group id
 * @param atime time since last access
 * @param mtime time since last modification
 * @param ctime time since last status change
 * @param data content of the file
 */
struct MyFsFile {
    char name[NAME_LENGTH];
    size_t size;
    st_mode mode;
    uid_t user_id;
    gid_t group_id;
    st_atime atime;
    st_mtime mtime;
    st_ctime ctime;
//TODO: In Blöcke einteilen
    // Achtung: Hat keinen Nullterminator!
    char* data;
};

#endif /* myfs_structs_h */
