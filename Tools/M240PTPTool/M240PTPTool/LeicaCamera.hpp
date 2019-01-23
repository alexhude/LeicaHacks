//
//  LeicaCamera.hpp
//  LeicaPTPTool
//
//  Created by Alexander Hude on 12/10/16.
//  Copyright © 2016 Alexander Hude. All rights reserved.
//

#pragma once

#include "libeasyptp/PTPBase.hpp"
#include <stdio.h>

using namespace EasyPTP;

class LeicaCamera : public PTPBase
{
public:
    LeicaCamera();
    LeicaCamera(IPTPComm * protocol);
    
    bool dumpDebugBuffer();
    
    bool debugCommand(char* command);
};
