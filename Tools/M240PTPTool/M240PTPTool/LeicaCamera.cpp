//
//  LeicaCamera.cpp
//  LeicaPTPTool
//
//  Created by Alexander Hude on 12/10/16.
//  Copyright © 2016 Alexander Hude. All rights reserved.
//

#include "LeicaCamera.hpp"
#include "libeasyptp/PTPContainer.hpp"
#include <string.h>

enum {
    kLeicaPTP_LE_GetDebugBuffer     = 0x900C,
    kLeicaPTP_LE_DebugCommandString = 0x900D,
};

enum {
    kLeicaPTP_Response_OK     = 0x2001,
};

LeicaCamera::LeicaCamera() : PTPBase()
{
    
}

LeicaCamera::LeicaCamera(IPTPComm * protocol) : PTPBase(protocol)
{
    
}

bool LeicaCamera::dumpDebugBuffer()
{
    PTPContainer cmd(PTPContainer::CONTAINER_TYPE_COMMAND, kLeicaPTP_LE_GetDebugBuffer);
    PTPContainer data;

    PTPContainer out_resp;
    PTPContainer out_data;

    this->ptp_transaction(cmd, data, true, out_resp, out_data, 1000);
    if (out_resp.code != kLeicaPTP_Response_OK)
        return false;
    
    int dumpLen = 0;
    char* dumpData = (char*)out_data.get_payload(&dumpLen);
    if (dumpLen == 0)
        return true;
    
    char* log = dumpData;
    while(dumpLen > 0) {
        while(*log == 0) {
            log++;
            dumpLen--;
        }

        if (dumpLen <= 0)
            break;
        
        int diff = 0;
        while(log[diff] != 0) diff++;
        
        if(diff > dumpLen) {
            char* stash = new char[diff + 1];
            memset(stash, 0, diff + 1);
            memcpy(stash, log, diff);
            printf("%s", stash); fflush(stdout);
            delete[] stash;
            break;
        }
        
        int written = printf("%s", log); fflush(stdout);
        dumpLen -= written;
        log += written;
    }
    
    delete[] dumpData;
    return true;
}

bool LeicaCamera::debugCommand(char* command)
{
    uint32_t payloadSize = (uint32_t)strlen(command) + 1;
    char* payload = new char[payloadSize];
    memset(payload, 0, payloadSize);
    
    strcpy(payload, command);
    
    PTPContainer cmd(PTPContainer::CONTAINER_TYPE_COMMAND, kLeicaPTP_LE_DebugCommandString);
    PTPContainer data(PTPContainer::CONTAINER_TYPE_DATA, kLeicaPTP_LE_DebugCommandString);
    data.set_payload(payload, payloadSize);
    delete[] payload;

    PTPContainer out_resp, out_data;
    this->ptp_transaction(cmd, data, false, out_resp, out_data, 1000);
    if (out_resp.code != kLeicaPTP_Response_OK) {
        printf("! Failed due to missing constrains\n");
        return false;
    }
    
    return true;
}
