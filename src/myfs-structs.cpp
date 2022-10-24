#include "myfs-structs.h"

size_t MyFsFile::getSize() {
    return size;
}

void MyFsFile::setSize(size_t size) {
    this->size = size;
}

void MyFsFile::setCTime(time_t cTime) {
    this->ctime = cTime;
}

time_t MyFsFile::getCTime() {
    return this->ctime;
}

void MyFsFile::setMTime(time_t mTime) {
    this->mtime = mTime;
}

time_t MyFsFile::getMTime() {
    return this->mtime;
}

void MyFsFile::setATime(time_t aTime) {
    this->atime = aTime;
}

time_t MyFsFile::getATime() {
    return this->atime;
}

void MyFsFile::setGID(gid_t gid) {
    this->group_id = gid;
}

gid_t MyFsFile::getGID() {
    return this->group_id;
}

void MyFsFile::setUID(uid_t uid) {
    this->user_id = uid;
}

uid_t MyFsFile::getUID() {
    return this->user_id;
}

mode_t MyFsFile::getMode() {
    return this->mode;
}

void MyFsFile::setMode(mode_t mode) {
    this->mode = mode;
}

char *MyFsFile::getName() {
    return this->name;
}

void MyFsFile::setName(char name[NAME_LENGTH]) {
    for(int i = 0; i< NAME_LENGTH; i++) {
        this->name[i] = name[i];
    }
}

/*!
 * @brief Searches the directory for a file named @param: searched
 * @param searched Searched File's name
 * @param result Optional Pointer of Searched File
 * @return bool found = true, not found = false
 */
bool MyFsDirectory::contains(const char *searched, MyFsFile *result) {
    for(auto &file : directory) {
        if(strcmp(file.name, searched) == 0) {
            result = &file;
            return true;
        }
    }
    return false;
}

/*!
 * Adds a File to current Directory
 * @param newFile
 * @return Success-code, 0 on success
 */
int MyFsDirectory::addFile(MyFsFile newFile) {
    return 0;
}
