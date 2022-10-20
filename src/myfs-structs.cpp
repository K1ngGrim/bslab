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

char *MyFsFile::Data() {
    return this->data;
}
