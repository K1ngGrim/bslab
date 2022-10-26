#include <cstring>
#include "myfs-structs.h"

/*!
 * @brief Searches the directory for a file named @param: searched
 * @param searched Searched File's name
 * @param result Optional Pointer of Searched File
 * @return bool found = true, not found = false
 */
bool MyFsDirectory::contains(const char *searched) {
    for(auto &file : directory) {
        if(strcmp(file.name, searched) == 0) {
            return true;
        }
    }
    return false;
}

/*!
 * Find a File in the current Directory
 * @param searched
 * @return index of found file, -1 if file does not exists
 */
int MyFsDirectory::find(const char *searched) {
    int index = 0;
    for(auto &file : directory) {
        if(strcmp(file.name, searched) == 0) {
            return index;
        }
        index++;
    }
    return -1;
}

/*!
 * Adds a File to current Directory
 * @param newFile
 * @return Success-code, 0 on success
 */
int MyFsDirectory::addFile(MyFsFile newFile) {
    int index = findFreeSpace();
    memcpy(&directory[index], &newFile, sizeof(MyFsFile));
    return 0;
}

/*!
 * Get a File of current Directory
 * @param searched Name of searched File
 * @return File if found, emtpy found if not
 */
MyFsFile MyFsDirectory::getFile(const char *searched) {

    MyFsFile fs = MyFsFile();
    if(int i = find(searched)> 0) {
        fs = directory[i];
    }
    return fs;
}

/*!
 * Find a free Space in Array
 * @return
 */
int MyFsDirectory::findFreeSpace() {

    int index = 0;
    for(auto &file : directory) {
        if(file.size == 0) {
            return index;
        }
        index++;
    }
    return -1;
}

int MyFsDirectory::deleteFile(const char *name) {

    if(contains(name)) {
        MyFsFile *file = &directory[find(name)];
        file->size = -1;
        strcpy(file->name, "");
        return 0;
    }else {
        ///File not found
    }


    return 0;
}
