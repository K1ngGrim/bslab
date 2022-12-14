//
// Created by Oliver Waldhorst on 20.03.20.
// Copyright © 2017-2020 Oliver Waldhorst. All rights reserved.
//

#include "myondiskfs.h"

// For documentation of FUSE methods see https://libfuse.github.io/doxygen/structfuse__operations.html

#undef DEBUG
#define DEBUG
#define DEBUG_METHODS
#define DEBUG_RETURN_VALUES

#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "macros.h"
#include "myfs.h"
#include "myfs-info.h"
#include "blockdevice.h"

int FAT[NUM_DATA_BLOCKS] = {};
MyFsFileOnMemory root[NUM_DIR_ENTRIES] = {};
MyFsSuperBlock myFsSuperBlock = MyFsSuperBlock();
int openFiles = 0;

/// @brief Constructor of the on-disk file system class.
///
/// You may add your own constructor code here.
MyOnDiskFS::MyOnDiskFS() : MyFS() {
    // create a block device object
    this->blockDevice= new BlockDevice(BLOCK_SIZE);

}

/// @brief Destructor of the on-disk file system class.
///
/// You may add your own destructor code here.
MyOnDiskFS::~MyOnDiskFS() {
    // free block device object
    delete this->blockDevice;

    // TODO: [PART 2] Add your cleanup code here

}

/// @brief Create a new file.
///
/// Create a new file with given name and permissions.
/// You do not have to check file permissions, but can assume that it is always ok to access the file.
/// \param [in] path Name of the file, starting with "/".
/// \param [in] mode Permissions for file access.
/// \param [in] dev Can be ignored.
/// \return 0 on success, -ERRNO on failure.
int MyOnDiskFS::fuseMknod(const char *path, mode_t mode, dev_t dev) {
    LOGM();

    int result = 0;

    for(auto i = 0; i < NUM_DIR_ENTRIES; i++) {
        if (strcmp(root[i].name, path + 1) == 0 || strlen(path + 1) > NAME_LENGTH) {
            result = -EEXIST;
            break;
        }
        if (strcmp(root[i].name, "") == 0) {
            int firstBlockIndex = findEmptyDataBlock();

            strcpy(root[i].name, path + 1);
            root[i].size = 0;
            root[i].mode = mode; //Necessary for generating a File
            root[i].group_id = getgid();
            root[i].user_id = getuid();
            root[i].mtime = time(nullptr);
            root[i].atime = time(nullptr);
            root[i].ctime = time(nullptr);
            root[i].firstBlock = firstBlockIndex;
            FAT[firstBlockIndex] = EOC_BLOCK;

            LOGF("Datei erstellt. Name: %s FirstBlock: %d", root[i].name, root[i].firstBlock);
            printf("Datei erstellt\n. Name: %s \nFirstBlock: %d\n Blockadresse: %d", root[i].name, root[i].firstBlock,
                   fatToAddress(root[i].firstBlock));
            SaveRoot();
            SaveFAT();
            break;
        }
    }

    RETURN(result)
}

/// @brief Delete a file.
///
/// Delete a file with given name from the file system.
/// You do not have to check file permissions, but can assume that it is always ok to access the file.
/// \param [in] path Name of the file, starting with "/".
/// \return 0 on success, -ERRNO on failure.
int MyOnDiskFS::fuseUnlink(const char *path) {
    LOGM();
    int result = 0;
    int index = findFile(path + 1);
    if (index >= 0) {
        root[index].size = -1;
        strcpy(root[index].name, "");
    } else {
        result = -ENOENT;
    }
    RETURN(result)
}

/// @brief Rename a file.
///
/// Rename the file with with a given name to a new name.
/// Note that if a file with the new name already exists it is replaced (i.e., removed
/// before renaming the file.
/// You do not have to check file permissions, but can assume that it is always ok to access the file.
/// \param [in] path Name of the file, starting with "/".
/// \param [in] newpath  New name of the file, starting with "/".
/// \return 0 on success, -ERRNO on failure.
int MyOnDiskFS::fuseRename(const char *path, const char *newpath) {
    LOGM();
    int result = 0;
    int index = findFile(path + 1);
    if (index >= 0) {
        strcpy(root[index].name, newpath + 1);
    } else {
        result = -ENOENT;
    }

    RETURN(result)
}

/// @brief Get file meta data.
///
/// Get the metadata of a file (user & group id, modification times, permissions, ...).
/// \param [in] path Name of the file, starting with "/".
/// \param [out] statbuf Structure containing the meta data, for details type "man 2 stat" in a terminal.
/// \return 0 on success, -ERRNO on failure.
int MyOnDiskFS::fuseGetattr(const char *path, struct stat *statbuf) {
    LOGM();

    statbuf->st_uid = getuid(); // The owner of the file/directory is the user who mounted the filesystem
    statbuf->st_gid = getgid(); // The group of the file/directory is the same as the group of the user who mounted the filesystem
    statbuf->st_atime = time(nullptr); // The last "a"ccess of the file/directory is right now
    statbuf->st_mtime = time(nullptr); // The last "m"odification of the file/directory is right now

    int ret = 0;

    if (strcmp(path, "/") == 0) {
        statbuf->st_mode = S_IFDIR | 0755;
        statbuf->st_nlink = 2; // Why "two" hardlinks instead of "one"? The answer is here: http://unix.stackexchange.com/a/101536
    } else {
        auto index = findFile(path+1);
        if(index >= 0) {
            statbuf->st_mode = root[index].mode;
            statbuf->st_size = (off_t) root[index].size;
            statbuf->st_uid = root[index].user_id;
            statbuf->st_gid = root[index].group_id;
            statbuf->st_ctime = root[index].ctime;
            statbuf->st_nlink = 1;
        }else {
            ret = -ENOENT;
        }
    }

    RETURN(ret);
}

/// @brief Change file permissions.
///
/// Set new permissions for a file.
/// You do not have to check file permissions, but can assume that it is always ok to access the file.
/// \param [in] path Name of the file, starting with "/".
/// \param [in] mode New mode of the file.
/// \return 0 on success, -ERRNO on failure.
int MyOnDiskFS::fuseChmod(const char *path, mode_t mode) {
    LOGM();
    int result = 0;
    int index = findFile(path + 1);
    if (index >= 0) {
        root[index].mode = mode;
        root[index].atime = root[index].mtime = time(nullptr);
    } else {
        result = -ENOENT;
    }

    RETURN(result)
}

/// @brief Change the owner of a file.
///
/// Change the user and group identifier in the meta data of a file.
/// You do not have to check file permissions, but can assume that it is always ok to access the file.
/// \param [in] path Name of the file, starting with "/".
/// \param [in] uid New user id.
/// \param [in] gid New group id.
/// \return 0 on success, -ERRNO on failure.
int MyOnDiskFS::fuseChown(const char *path, uid_t uid, gid_t gid) {
    LOGM();
    int result = 0;
    int index = findFile(path + 1);
    if (index >= 0) {
        root[index].user_id = uid;
        root[index].group_id = gid;
        root[index].atime = root[index].mtime = time(nullptr);
    } else {
        result = -ENOENT;
    }

    RETURN(result)
}

/// @brief Open a file.
///
/// Open a file for reading or writing. This includes checking the permissions of the current user and incrementing the
/// open file count.
/// You do not have to check file permissions, but can assume that it is always ok to access the file.
/// \param [in] path Name of the file, starting with "/".
/// \param [out] fileInfo Can be ignored in Part 1
/// \return 0 on success, -ERRNO on failure.
int MyOnDiskFS::fuseOpen(const char *path, struct fuse_file_info *fileInfo) {
    LOGM();
    int result = 0;
    int index = findFile(path + 1);
    if (index >= 0) {
        openFiles++;
        fileInfo->fh = index;
        root[index].atime = time(nullptr);
    } else {
        result = -ENOENT;
    }

    RETURN(result)
}

/// @brief Read from a file.
///
/// Read a given number of bytes from a file starting form a given position.
/// You do not have to check file permissions, but can assume that it is always ok to access the file.
/// Note that the file content is an array of bytes, not a string. I.e., it is not (!) necessarily terminated by '\0'
/// and may contain an arbitrary number of '\0'at any position. Thus, you should not use strlen(), strcpy(), strcmp(),
/// ... on both the file content and buf, but explicitly store the length of the file and all buffers somewhere and use
/// memcpy(), memcmp(), ... to process the content.
/// \param [in] path Name of the file, starting with "/".
/// \param [out] buf The data read from the file is stored in this array. You can assume that the size of buffer is at
/// least 'size'
/// \param [in] size Number of bytes to read
/// \param [in] offset Starting position in the file, i.e., number of the first byte to read relative to the first byte of
/// the file
/// \param [in] fileInfo Can be ignored in Part 1
/// \return The Number of bytes read on success. This may be less than size if the file does not contain sufficient bytes.
/// -ERRNO on failure.
int MyOnDiskFS::fuseRead(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo) {
    LOGM();
    this->blockDevice->read(fatToAddress(root[fileInfo->fh].firstBlock), buf);
    RETURN(0);
}

/// @brief Write to a file.
///
/// Write a given number of bytes to a file starting at a given position.
/// You do not have to check file permissions, but can assume that it is always ok to access the file.
/// Note that the file content is an array of bytes, not a string. I.e., it is not (!) necessarily terminated by '\0'
/// and may contain an arbitrary number of '\0'at any position. Thus, you should not use strlen(), strcpy(), strcmp(),
/// ... on both the file content and buf, but explicitly store the length of the file and all buffers somewhere and use
/// memcpy(), memcmp(), ... to process the content.
/// \param [in] path Name of the file, starting with "/".
/// \param [in] buf An array containing the bytes that should be written.
/// \param [in] size Number of bytes to write.
/// \param [in] offset Starting position in the file, i.e., number of the first byte to read relative to the first byte of
/// the file.
/// \param [in] fileInfo Can be ignored in Part 1 .
/// \return Number of bytes written on success, -ERRNO on failure.
int MyOnDiskFS::fuseWrite(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo) {
    LOGM();

    char *content = new char[size];
    readData(root[fileInfo->fh], content);
    fuseTruncate(path, offset+size);
    memcpy(content+offset, buf, size);
    writeData(root[fileInfo->fh], content, root[fileInfo->fh].size);
    this->blockDevice->write(fatToAddress(root[fileInfo->fh].firstBlock), content);

    free(content);
    RETURN((int) size);
}

/// @brief Close a file.
///
/// \param [in] path Name of the file, starting with "/".
/// \param [in] File handel for the file set by fuseOpen.
/// \return 0 on success, -ERRNO on failure.
int MyOnDiskFS::fuseRelease(const char *path, struct fuse_file_info *fileInfo) {
    LOGM();

    fileInfo->fh = NULL;
    openFiles--;
    SaveFAT();
    SaveRoot();

    RETURN(0);
}

/// @brief Truncate a file.
///
/// Set the size of a file to the new size. If the new size is smaller than the old size, spare bytes are removed. If
/// the new size is larger than the old size, the new bytes may be random.
/// You do not have to check file permissions, but can assume that it is always ok to access the file.
/// \param [in] path Name of the file, starting with "/".
/// \param [in] newSize New size of the file.
/// \return 0 on success, -ERRNO on failure.
int MyOnDiskFS::fuseTruncate(const char *path, off_t newSize) {
    LOGM();

    int fileIndex = findFile(path+1);
    int claimedBlocks = (root[fileIndex].size / BLOCK_SIZE) + 1;
    int neededBlocks = (newSize / BLOCK_SIZE ) + 1;
    //newSize > oldSize
    if(root[fileIndex].size < newSize) {
        if(claimedBlocks == neededBlocks) {
            root[fileIndex].size = newSize;
        }else {
            //Neue Blöcke reservieren: neededBlocks - claimedBlocks = wieviele blöcke wir zusätzlich brauchen

            int lastBlock = findEOC(root[fileIndex].firstBlock);
            for(int i = 0; i < neededBlocks - claimedBlocks; i++) {
                int newBlock = findEmptyDataBlock();
                FAT[lastBlock] = newBlock;
                lastBlock = newBlock;
            }
            root[fileIndex].size = newSize;
        }
    }else if(root[fileIndex].size > newSize) {
        if(claimedBlocks == neededBlocks) {
            root[fileIndex].size = newSize;
            return 0;
        }else {
            //EOC verschieben und rest befreien
            int block = root[fileIndex].firstBlock;
            for(int i = 1; i < neededBlocks; i++) {
                block = FAT[block];
            }
            int blockToFree = FAT[block];
            FAT[block] = EOC_BLOCK;
            int nextBlock = -1;
            do {
                if(nextBlock != -1) blockToFree = nextBlock;
                nextBlock = FAT[blockToFree];
                FAT[blockToFree] = EMPTY_BLOCK;
            }while(nextBlock != EOC_BLOCK);
        }

    }

    SaveFAT();
    SaveRoot();
    RETURN(0);
}

/// @brief Truncate a file.
///
/// Set the size of a file to the new size. If the new size is smaller than the old size, spare bytes are removed. If
/// the new size is larger than the old size, the new bytes may be random. This function is called for files that are
/// open.
/// You do not have to check file permissions, but can assume that it is always ok to access the file.
/// \param [in] path Name of the file, starting with "/".
/// \param [in] newSize New size of the file.
/// \param [in] fileInfo Can be ignored in Part 1.
/// \return 0 on success, -ERRNO on failure.
int MyOnDiskFS::fuseTruncate(const char *path, off_t newSize, struct fuse_file_info *fileInfo) {
    LOGM();

    root[fileInfo->fh].size = newSize;

    RETURN(0);
}

/// @brief Read a directory.
///
/// Read the content of the (only) directory.
/// You do not have to check file permissions, but can assume that it is always ok to access the directory.
/// \param [in] path Path of the directory. Should be "/" in our case.
/// \param [out] buf A buffer for storing the directory entries.
/// \param [in] filler A function for putting entries into the buffer.
/// \param [in] offset Can be ignored.
/// \param [in] fileInfo Can be ignored.
/// \return 0 on success, -ERRNO on failure.
int MyOnDiskFS::fuseReaddir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileInfo) {
    LOGM();

    filler(buf, ".", nullptr, 0); // Current Directory
    filler(buf, "..", nullptr, 0); // Parent Directory
    struct stat stat = {};

    if (strcmp(path, "/") ==
        0) // If the user is trying to show the files/directories of the root directory show the following
    {
        for(auto i = 0; i < NUM_DIR_ENTRIES; i++) {
            if (strcmp(root[i].name, "") != 0 && root[i].size != -1) {
                fuseGetattr(path, &stat);
                filler(buf, root[i].name, &stat, 0);
            }
        }
    }
    RETURN(0)
}

/// Initialize a file system.
///
/// This function is called when the file system is mounted. You may add some initializing code here.
/// \param [in] conn Can be ignored.
/// \return 0.
void* MyOnDiskFS::fuseInit(struct fuse_conn_info *conn) {
    // Open logfile
    this->logFile= fopen(((MyFsInfo *) fuse_get_context()->private_data)->logFile, "w+");

    if(this->logFile == NULL) {
        fprintf(stderr, "ERROR: Cannot open logfile %s\n", ((MyFsInfo *) fuse_get_context()->private_data)->logFile);
    } else {
        // turn of logfile buffering
        setvbuf(this->logFile, NULL, _IOLBF, 0);

        LOG("Starting logging...\n");

        LOG("Using on-disk mode");

        LOGF("Container file name: %s", ((MyFsInfo *) fuse_get_context()->private_data)->contFile);



        int ret= this->blockDevice->open(((MyFsInfo *) fuse_get_context()->private_data)->contFile);

        if(ret >= 0) {
            LOG("Container file does exist, reading");
            /*!
             * Lesen der Blöcke Superblock und Fat. Einlesen in die vorhandenen Arrays
             */
            char* buffer = new char[BLOCK_SIZE];

            //Block mit der nummer 0 lesen
            this->blockDevice->read(0, buffer);
            //Block 0 in superblock schreiben
            memcpy(&myFsSuperBlock, buffer, sizeof(myFsSuperBlock));
            /* Lesen des FATS
             * Wir lesen von superBlock.Fat aus jeden Block einzeln in Buffer.
             * Von Buffer kopieren wir diesen gelesenen Block in fatBuffer, der so groß ist, wie das Fat int Array
             * Am Ende kopieren wir alles vom FatBuffer in den richtigen FAT
             */

            int pointer = 0;
            for(int i = myFsSuperBlock.FAT; i < myFsSuperBlock.root; i++, pointer++) {
                this->blockDevice->read(i, buffer);
                memcpy(FAT+pointer, buffer, BLOCK_SIZE);
            }

            int bufferSize = sizeof(MyFsFileOnMemory) * NUM_DIR_ENTRIES;
            char* rootBuffer = new char[bufferSize];
            int size = sizeof(MyFsFileOnMemory)*NUM_DIR_ENTRIES/BLOCK_SIZE;
            for(int i = 0; i < size; i++) {
                this->blockDevice->read(i+myFsSuperBlock.root, &rootBuffer[i*BLOCK_SIZE]);
            }
            memcpy(&root, rootBuffer, bufferSize);

            free(rootBuffer);
        } else if(ret == -ENOENT) {
            LOG("Container file does not exist, creating a new one");

            ret = this->blockDevice->create(((MyFsInfo *) fuse_get_context()->private_data)->contFile);

            if (ret >= 0) {
                char* buffer = new char[BLOCK_SIZE];
                MyFsSuperBlock fs {};
                memcpy(buffer, &myFsSuperBlock, BLOCK_SIZE);

                this->blockDevice->write(0, buffer);


                free(buffer);

                for(int i = 0; i < NUM_DATA_BLOCKS; ++i) {
                    FAT[i]=EMPTY_BLOCK;
                }

                char* b = new char[sizeof(int)*NUM_DATA_BLOCKS];
                memcpy(b, FAT, sizeof(int)*NUM_DATA_BLOCKS);
                for(int i = 0; i< NUM_DATA_BLOCKS; i++) {
                    this->blockDevice->write(i+myFsSuperBlock.FAT, b+i);
                }
                free(b);

                //Root schreiben
                //char* r = new char[sizeof(MyFsFileOnMemory)*NUM_DIR_ENTRIES];

                int bufferSize = sizeof(MyFsFileOnMemory) * NUM_DIR_ENTRIES;
                buffer = new char[bufferSize];
                memcpy(buffer, root, bufferSize);

                int blocksToWrite = ((int) bufferSize / BLOCK_SIZE) + 1;
                LOGF("%d", myFsSuperBlock.root);
                for(int i = myFsSuperBlock.root; i < myFsSuperBlock.root + blocksToWrite; i++) {
                    this->blockDevice->write(i, buffer+(BLOCK_SIZE*i));
                }

                free(buffer);
            }
        }

        if(ret < 0) {
            LOGF("ERROR: Access to container file failed with error %d", ret);
        }
     }

    return 0;
}

/// @brief Clean up a file system.
///
/// This function is called when the file system is unmounted. You may add some cleanup code here.
void MyOnDiskFS::fuseDestroy() {
    LOGM();

    // TODO: [PART 2] Implement this!

}

// TODO: [PART 2] You may add your own additional methods here!

int MyOnDiskFS::findFile(const char *nameToFind) {
    int index = 0;
    for(auto i = 0; i < NUM_DIR_ENTRIES; i++, index++) {
        if (strcmp(root[i].name, nameToFind) == 0) {
            return index;
        }
    }
    return -1;
}

/*!
 * Returns FAT Index with EOC Flag
 * @param firstBlock
 * @return
 */
int MyOnDiskFS::findEOC(int firstBlock) {

    int block = firstBlock;
    while(FAT[block] != EOC_BLOCK) block = FAT[block];

    return block;
}

int MyOnDiskFS::findEmptyDataBlock() {
    int index = -1;
    for(auto i = 0; i< NUM_DATA_BLOCKS; i++) {
        if(FAT[i] == EMPTY_BLOCK) {
            index = i;
            break;
        }
    }
    return index;
}

/*!
 * @brief Saves Root, FAT
 * @return 0 on success
 */
int MyOnDiskFS::Save() {

    //Saves FAT

    char ro[sizeof(MyFsFileOnMemory)*NUM_DIR_ENTRIES] = {};
    memcpy(ro, root, sizeof(MyFsFileOnMemory)*NUM_DIR_ENTRIES);
    int size = sizeof(MyFsFileOnMemory)*NUM_DIR_ENTRIES/BLOCK_SIZE;
    for(int i = 0; i<size; i++) {
        this->blockDevice->write(i+myFsSuperBlock.root, &ro[i*BLOCK_SIZE]);
    }
    return 0;
}

int MyOnDiskFS::SaveRoot() {
    char ro[sizeof(MyFsFileOnMemory)*NUM_DIR_ENTRIES] = {};
    memcpy(ro, root, sizeof(MyFsFileOnMemory)*NUM_DIR_ENTRIES);
    int size = sizeof(MyFsFileOnMemory)*NUM_DIR_ENTRIES/BLOCK_SIZE;
    for(int i = 0; i<size; i++) {
        this->blockDevice->write(i+myFsSuperBlock.root, &ro[i*BLOCK_SIZE]);
    }
    return 0;
}

int MyOnDiskFS::SaveFAT() {
    char* b = new char[sizeof(int)*NUM_DATA_BLOCKS];
    memcpy(b, FAT, sizeof(int)*NUM_DATA_BLOCKS);
    for(int i = 0; i< NUM_DATA_BLOCKS; i++) {
        this->blockDevice->write(i+myFsSuperBlock.FAT, b+(i*BLOCK_SIZE));
    }
    free(b);
}

int MyOnDiskFS::SaveData(int dataBlockIndex, char*buffer) {
    return this->blockDevice->write(dataBlockIndex+myFsSuperBlock.data, buffer);
}

int MyOnDiskFS::readData(MyFsFileOnMemory fileInfo, char*buffer) {
    if(fileInfo.firstBlock < 0) return -EEXIST;

    int block = fileInfo.firstBlock;
    int step = 0;
    do {
        char* readBuffer = new char[512];
        this->blockDevice->read(fatToAddress(block), readBuffer);
        memcpy(buffer + (step*BLOCK_SIZE), readBuffer, BLOCK_SIZE);
        step+=1;
        block = FAT[block];
    }while(block != EOC_BLOCK);

    return 0;
}


int MyOnDiskFS::writeData(MyFsFileOnMemory fileInfo, char*buffer, int size) {
    int result = 0;
    int numBlocks = (size/BLOCK_SIZE)+1;
    int block = fileInfo.firstBlock;
    for(int i = 0; i < numBlocks; i++) {
        result = SaveData(block, buffer+(BLOCK_SIZE*i));
        if(result != 0) return -EEXIST;
        block = FAT[block];
        if(block == EOC_BLOCK) break;
    }
    return 0;
}

int MyOnDiskFS::fatToAddress(int fatIndex) {
    return fatIndex + myFsSuperBlock.data;
}



// DO NOT EDIT ANYTHING BELOW THIS LINE!!!

/// @brief Set the static instance of the file system.
///
/// Do not edit this method!
void MyOnDiskFS::SetInstance() {
    MyFS::_instance= new MyOnDiskFS();
}
