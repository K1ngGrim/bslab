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
public:
    char name[NAME_LENGTH] = "";
    size_t size = -1;
    mode_t mode;
    uid_t user_id = getuid();
    gid_t group_id = getgid();
    nlink_t link = 1;
    time_t atime = time(NULL);
    time_t mtime = time(NULL);
    time_t ctime = time(NULL);

    // Achtung: Hat keinen Nullterminator!
    char* data;
};

class MyFsDirectory {
public:

    /*!
     * @brief Main directory with space of NUM_DIR_ENTRIES
     */
    MyFsFile directory[NUM_DIR_ENTRIES] = {};

    /*!
     * @brief Add a file to current directory
     * @param newFile File Object with data
     * @return Success Code
     */
    int addFile(MyFsFile newFile);

    /*!
     * @brief Check if the directory contains a File by name
     * @param searched Name of the File
     * @return true if found, false else
     */
    bool contains(const char searched[]);

    /*!
     * @brief Get a File with a certain name
     * @param searched
     * @return the File if it exists, empty File else
     */
    MyFsFile getFile(const char searched[]);

    /*!
     * @brief Find a File by name
     * @param searched: name of the File
     * @return index of File in array
     */
    int find(const char searched[]);

    /*!
     * @brief Find first free Space to write a new File in it.
     * @return index in Array
     */
    int findFreeSpace();

    /*!
     * @brief Soft-deleting File. Setting size to -1 and name to "" for overwriting
     * @param name: Name of the File without starting /
     * @return Success code as int for Success or Errors
     */
    int deleteFile(const char name[]);

    /*!
     * @brief Rename a File, Deleting the File named newName if its already exists.
     * @param oldName current name of the File
     * @param newName new name of the File
     * @return Success Code
     */
    int renameFile(const char oldName[], const char newName[]);

};

#endif /* myfs_structs_h */
