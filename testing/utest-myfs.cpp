//
//  test-myfs.cpp
//  testing
//
//  Created by Oliver Waldhorst on 15.12.17.
//  Copyright © 2017 Oliver Waldhorst. All rights reserved.
//

#include <sstream>
#include "../catch/catch.hpp"

#include "myondiskfs.h"
#include "tools.hpp"
#include "myfs.h"

// TODO: Implement your helper functions here!

MyOnDiskFS *testFs;
void uTestCreateOnDiskFSMock();

using namespace std;


TEST_CASE("UT-1.01", "[findFile]") {
    testFs = new MyOnDiskFS();
    uTestCreateOnDiskFSMock();
    printf("Testcase 1.1: Search a not existing File\n");

    REQUIRE(testFs->findFile("Benson") == -1);

    printf("Testcase 1.2: Search a File named \"File 1\"\n");

    REQUIRE(testFs->findFile("File 1") == 1);

    printf("Testcase 1.3: Search a File named \"File 8\"\n");

    REQUIRE(testFs->findFile("File 8") == 8);
}

TEST_CASE("UT-1.02", "[findEmptyDataBlock]") {
    testFs = new MyOnDiskFS();
    uTestCreateOnDiskFSMock();
    int used[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 20, 30};
    printf("Testcase 2.1: Find Empty Data-block in FAT\n");

    int index = testFs->findEmptyDataBlock();
    for(int i = 0; i< (sizeof(used)/sizeof(int)); i++) {
        REQUIRE_FALSE(used[i] == index);
    }
}

TEST_CASE("UT-1.03", "[findEOC]") {
    printf("Testcase 3.1: Find End-of-Cluster Block in FAT for Startblock 1\n");
    REQUIRE(testFs->findEOC(1) == 1);

    printf("Testcase 3.2: Find End-of-Cluster Block in FAT for Startblock 10\n");
    REQUIRE(testFs->findEOC(10) == 30);
}


void uTestCreateOnDiskFSMock() {
    MyFsFileOnMemory* file = new MyFsFileOnMemory();

    for(int i = 0; i < NUM_DATA_BLOCKS; ++i) {
        testFs->FAT[i]=EMPTY_BLOCK;
    }

    for(int i =1; i<= 10; i++) {
        const char *name = "File ";
        std::stringstream ss;
        ss << name << i;
        strcpy(file->name, ss.str().c_str());

        file->firstBlock = i;
        testFs->FAT[i] = EOC_BLOCK;


        file->size = i*10;
        memcpy((testFs->root+i), file, sizeof(MyFsFileOnMemory));
    }

    //Init a test FAT

    testFs->setFatEntry(10,20);
    testFs->setFatEntry(20, 30);
    testFs->setFatEntry(30, EOC_BLOCK);

    free(file);
}



