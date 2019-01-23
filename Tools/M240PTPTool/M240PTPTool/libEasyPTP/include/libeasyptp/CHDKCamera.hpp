/**
 * Copyright 2013 Bobby Graese <bobby.graese@gmail.com>
 * 
 * This file is part of libEasyPTP.
 *
 *  libEasyPTP is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  libEasyPTP is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with libEasyPTP.  If not, see
 *  <http://www.gnu.org/licenses/>.
 */

#ifndef LIBEASYPTP_CHDKCAMERA_H_
#define LIBEASYPTP_CHDKCAMERA_H_

#include <string>
#include <vector>
#include "libeasyptp/PTPBase.hpp"

namespace EasyPTP
{

class PTPContainer;
class LVData;
class IPTPComm;

// Picked out of CHDK source in a header we don't want to include

enum CHDK_PTP_RESP
{
    CHDK_PTP_RC_OK = 0x2001,
    CHDK_PTP_RC_GeneralError = 0x2002,
    CHDK_PTP_RC_ParameterNotSupported = 0x2006,
    CHDK_PTP_RC_InvalidParameter = 0x201D
};

class CHDKCamera : public PTPBase
{
    static uint8_t * _pack_file_for_upload(uint32_t * out_size, const std::string local_filename, const std::string remote_filename);
public:
    CHDKCamera();
    CHDKCamera(IPTPComm * protocol);
    float get_chdk_version(void);
    uint32_t check_script_status(void);
    uint32_t execute_lua(const std::string script, uint32_t * script_error, const bool block = false);
    void read_script_message(PTPContainer& out_data, PTPContainer& out_resp);
    uint32_t write_script_message(const std::string message, const uint32_t script_id = 0);
    bool upload_file(const std::string local_filename, const std::string remote_filename, int timeout = 0);
    char * download_file(const std::string filename, const int timeout);
    void get_live_view_data(LVData& data_out, const bool liveview = true, const bool overlay = false, const bool palette = false);
    std::vector<std::string> _wait_for_script_return(const int timeout);
};

}

#endif /* LIBEASYPTP_CHDKCAMERA_H_ */
