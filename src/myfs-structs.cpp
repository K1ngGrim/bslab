#include <cstring>
#include <cerrno>
#include "myfs-structs.h"

namespace MyFs_Namespace {
    int openFiles = 0;
    int FAT[NUM_DATA_BLOCKS] = {};
    MyFsFileOnMemory root[NUM_DIR_ENTRIES] = {};
    MyFsSuperBlock myFsSuperBlock = MyFsSuperBlock();
}