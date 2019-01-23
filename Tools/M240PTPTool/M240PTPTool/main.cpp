//
//  main.cpp
//  LeicaPTPTool
//
//  Created by Alexander Hude on 12/10/16.
//  Copyright © 2016 Alexander Hude. All rights reserved.
//

#include <iostream>
#include <unistd.h>

#include "libeasyptp/PTPUSB.hpp"
#include "LeicaCamera.hpp"

int main(int argc, const char * argv[]) {
    
    printf("Leica Extended PTP interface PoC\n");
    
    PTPUSB* ptpusb = new PTPUSB();
    ptpusb->connect_to_first();
    
    if (!ptpusb->is_open()) {
        printf("Leica PTP not found!\n");
        return -1;
    }
    
    LeicaCamera* lc = new LeicaCamera(ptpusb);
    
    char command[600];
    while(1) {
        usleep(500*1000);
        memset(command, 0, sizeof(command));

        printf("M240> "); fflush(stdout);
        fgets (command, sizeof(command), stdin);
        command[strlen(command)-1] = 0;
        
        if(command[0] == 0)
            continue;
        
        if (strcmp(command, "exit") == 0)
            break;

        if (strcmp(command, "flush") == 0) {
            lc->dumpDebugBuffer();
            continue;
        }

        lc->debugCommand(command);
        sleep(1);
        lc->dumpDebugBuffer();
    }

    delete lc;
    delete ptpusb;
    
    return 0;
}
