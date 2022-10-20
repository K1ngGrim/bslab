//
///  myfs-structs.h
///  myfs
//
///  Created by Oliver Waldhorst on 07.09.17.
///  Copyright © 2017 Oliver Waldhorst. All rights reserved.
///

#include <sys/stat.h>
#include <cstdio>

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
private:
    char name[NAME_LENGTH];
    size_t size;
    mode_t mode;
    uid_t user_id;
    gid_t group_id;
    time_t atime;
    time_t mtime;
    time_t ctime;


//TODO: In Blöcke einteilen Später
    // Achtung: Hat keinen Nullterminator!
    char* data;

public:
    size_t getSize();
    void setSize(size_t size);

    mode_t getMode();
    void setMode(mode_t mode);

    uid_t getUID();
    void setUID(uid_t uid);

    gid_t getGID();
    void setGID(gid_t gid);

    time_t getATime();
    void setATime(time_t aTime);

    time_t getMTime();
    void setMTime(time_t mTime);

    time_t getCTime();
    void setCTime(time_t cTime);

    char* Data();
};

#endif /* myfs_structs_h */
