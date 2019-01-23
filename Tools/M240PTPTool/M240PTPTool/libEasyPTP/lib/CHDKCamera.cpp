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

/**
 * @file CHDKCamera.cpp
 * 
 * @brief A class for communicating specifically with cameras running CHDK
 * 
 * This class is really what this whole library is designed for: communication
 * with cameras running CHDK.  This file defines the multiple convenience
 * functions that make communicating with CHDK simple.
 */

#include <cstring>
#include <fstream>
// Needed for usleep() in script wait
#include <unistd.h>
#include <stdint.h>
#include <sys/time.h>

#include "libeasyptp/PTPErrors.hpp"
#include "libeasyptp/CHDKCamera.hpp"
#include "libeasyptp/PTPContainer.hpp"
#include "libeasyptp/LVData.hpp"
#include "libeasyptp/chdk/ptp.h"

namespace EasyPTP
{

/**
 * Creates an empty \c CHDKCamera, without connecting to a camera.
 */
CHDKCamera::CHDKCamera() : PTPBase()
{

}

/**
 * Creates a \c CHDKCamera and connects to the \c libusb_device \a dev.
 *
 * @param[in] dev The \c libusb_device to connect to.
 * @see PTPBase::PTPBase(libusb_device * dev)
 */
CHDKCamera::CHDKCamera(IPTPComm * protocol) : PTPBase(protocol)
{

}

/**
 * Retrieve the version of CHDK that this \c CHDKCamera is connected to.
 * 
 * @note Assumes the minor version is one digit long.
 * @return The CHDK version number.
 */
float CHDKCamera::get_chdk_version(void)
{
    PTPContainer cmd(PTPContainer::CONTAINER_TYPE_COMMAND, 0x9999);
    cmd.add_param(PTP_CHDK_Version);

    PTPContainer out_resp, data, out_data;
    this->ptp_transaction(cmd, data, false, out_resp, out_data);
    // param 1 is four bytes of major version
    // param 2 is four bytes of minor version
    float out;
    unsigned char * payload;
    int payload_size;
    uint32_t major = 0, minor = 0;
    payload = out_resp.get_payload(&payload_size);
    if (payload_size >= 8)
    { // Need at least 8 bytes in the payload
        std::memcpy(&major, payload, 4); // Copy first four bytes into major
        std::memcpy(&minor, payload + 4, 4); // Copy next four bytes into minor
    }
    delete[] payload;

    out = major + minor / 10.0; // This assumes that the minor version is one digit long
    return out;
}

/**
 * Checks the status of the currently running script.
 *
 * @return The current script status, a member of CHDK_SCRIPT_STATUS
 */
uint32_t CHDKCamera::check_script_status(void)
{
    PTPContainer cmd(PTPContainer::CONTAINER_TYPE_COMMAND, 0x9999);
    cmd.add_param(PTP_CHDK_ScriptStatus);

    PTPContainer out_resp, data, out_data;
    this->ptp_transaction(cmd, data, true, out_resp, out_data);

    return out_resp.get_param_n(0);
}

/**
 * Asks CHDK to execute the lua script given by \c script.
 *
 * @param[in] script The LUA script to execute.
 * @param[out] script_error The error code returned by the script, if blocking.
 * @param[in] block Whether or not to block execution until the script has returned.
 * @return The first parameter in the PTP response (?)
 * @todo Finish blocking code, allow timeout input
 */
uint32_t CHDKCamera::execute_lua(const std::string script, uint32_t * script_error, const bool block)
{
    PTPContainer cmd(PTPContainer::CONTAINER_TYPE_COMMAND, 0x9999);
    cmd.add_param(PTP_CHDK_ExecuteScript);
    cmd.add_param(PTP_CHDK_SL_LUA);

    PTPContainer data(PTPContainer::CONTAINER_TYPE_DATA, 0x9999);
    data.set_payload(script.c_str(), script.length() + 1);

    PTPContainer out_resp, out_data;
    this->ptp_transaction(cmd, data, false, out_resp, out_data);

    uint32_t out = -1;
    unsigned char * payload;
    int payload_size;
    payload = out_resp.get_payload(&payload_size);

    if (block)
    {
        //printf("TODO: Blocking code");
        this->_wait_for_script_return(5);
    }
    else
    {
        if (payload_size >= 8)
        { // Need at least 8 bytes in the payload
            std::memcpy(&out, payload, 4);
            if (script_error != NULL)
            {
                std::memcpy(script_error, payload + 4, 4);
            }
        }
    }
    delete[] payload;

    return out;
}

/**
 * @brief Read the current script message from CHDK
 *
 * Simply returns the \c PTPContainer for handling by the caller.
 *
 * @param[out] out_resp \c PTPContainer containing the response from the PTP transaction.
 * @param[out] out_data \c PTPContainer containing the data from the PTP transaction.
 * 
 * @todo Convert to a string and return actual message?
 */
void CHDKCamera::read_script_message(PTPContainer& out_resp, PTPContainer& out_data)
{
    PTPContainer cmd(PTPContainer::CONTAINER_TYPE_COMMAND, 0x9999);
    cmd.add_param(PTP_CHDK_ReadScriptMsg);
    cmd.add_param(PTP_CHDK_SL_LUA);

    PTPContainer data;
    this->ptp_transaction(cmd, data, true, out_resp, out_data);
    // We'll just let the caller deal with the data
}

/**
 * @brief Write a message to the script running on CHDK
 *
 * @param[in] message The message to send to the script.
 * @param[in] script_id (optional) The ID of the script to send the message to.
 * @return The first parameter from the PTP response.
 */
uint32_t CHDKCamera::write_script_message(const std::string message, const uint32_t script_id)
{
    PTPContainer cmd(PTPContainer::CONTAINER_TYPE_COMMAND, 0x9999);
    cmd.add_param(PTP_CHDK_WriteScriptMsg);
    cmd.add_param(script_id);

    PTPContainer data(PTPContainer::CONTAINER_TYPE_DATA, 0x9999);
    data.set_payload(message.c_str(), message.length());

    PTPContainer out_resp, out_data;
    this->ptp_transaction(cmd, data, false, out_resp, out_data);

    uint32_t out = -1;
    unsigned char * payload;
    int payload_size;
    payload = out_resp.get_payload(&payload_size);

    if (payload_size >= 4)
    { // Need four bytes of uint32_t response
        std::memcpy(&out, payload, 4);
    }
    delete[] payload;

    return out;
}

/**
 * @brief Retrieve live view data from CHDK
 *
 * Returns selected frame buffers from CHDK in an \c LVData object.  By default, only live view data
 * is returned, but overlay and palette can optionally be returned, also.  The \c LVData object can
 * then be used to retrieve and manipulate the live view data.
 *
 * @param[out] data_out The address of an LVData object which will be populated with the requested data
 * @param[in]  liveview True to return the live view frame buffer
 * @param[in]  overlay  True to return the overlay frame buffer
 * @param[in]  palette  True to return the palette for the overlay
 * @see LVData, http://chdk.wikia.com/wiki/Frame_buffers
 */
void CHDKCamera::get_live_view_data(LVData& data_out, const bool liveview, const bool overlay, const bool palette)
{
    uint32_t flags = 0;
    if (liveview) flags |= LV_TFR_VIEWPORT;
    if (overlay) flags |= LV_TFR_BITMAP;
    if (palette) flags |= LV_TFR_PALETTE;

    PTPContainer cmd(PTPContainer::CONTAINER_TYPE_COMMAND, 0x9999);
    cmd.add_param(PTP_CHDK_GetDisplayData);
    cmd.add_param(flags);

    PTPContainer data, out_resp, out_data;
    this->ptp_transaction(cmd, data, true, out_resp, out_data);

    int payload_size;
    unsigned char * payload = out_data.get_payload(&payload_size);

    data_out.read(payload, payload_size); // The LVData class will completely handle the LV data

    delete[] payload;
}

/**
 * @brief Block until the currently running script returns a value
 *
 * This function will poll the camera every 50 ms for script messages.  If a
 * script is currently still running, it will continue to poll until all scripts
 * are done running.  All read messages are returned when all scripts are done
 * running.
 *
 * @todo Determine a method for returning the messages
 *
 * @param[in] timeout The maximum amount of time to let this function run for
 * @return All read script messages.
 */
std::vector<std::string> CHDKCamera::_wait_for_script_return(const int timeout)
{
    //int msg_count = 1;
    std::vector<std::string> msgs;
    struct timeval time;
    long t_start;
    long t_end;
    uint32_t status;

    gettimeofday(&time, NULL);
    t_start = (time.tv_sec * 1000) + (time.tv_usec / 1000);

    while (1)
    {
        status = this->check_script_status();

        if (status & PTP_CHDK_SCRIPT_STATUS_RUN)
        { // If a script is running
            // Sleep for 50 ms
            usleep(50 * 1000);
            gettimeofday(&time, NULL);
            t_end = (time.tv_sec * 1000) + (time.tv_usec / 1000);
            if (timeout > 0 && timeout > (t_end - t_start))
            {
                throw ERR_TIMEOUT;
            }
        }
        else if (status & PTP_CHDK_SCRIPT_STATUS_MSG)
        {
            // TODO: Read script message, determine how to return
        }
        else if (status == 0)
        {
            break;
        }
        else
        {
            throw ERR_INVALID_RESPONSE;
        }
    }

    return msgs;
}

/**
 * @brief Reads in a file and packs data for uploading.
 * 
 * From CHDK source code, the correct format for the uploaded file is:
 *  -# Four bytes of length of filename
 *  -# Filename
 *  -# Contents of file
 * 
 * @warning The caller is responsible for free()ing the memory allocated by this
 *          function.
 * 
 * @param[out] out_size The size of the resulting data structure
 * @param[in] local_filename The path and name of the local file to be read
 * @param[in] remote_filename The path and name of the location on the camera to place the file
 * @return A pointer to the first byte of the resulting data
 * @see CHDKCamera::upload_file
 */
uint8_t * CHDKCamera::_pack_file_for_upload(uint32_t * out_size, const std::string local_filename, const std::string remote_filename)
{
    uint32_t file_size;
    uint8_t * out;
    int name_length;

    name_length = remote_filename.length();

    std::ifstream stream_local(local_filename.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
    // Open file for reading, binary type of file, place pointer at end of file

    file_size = stream_local.tellg(); // Retrives the position of the input stream
    // Since we asked to open the file at the end, this is the length of the file
    stream_local.seekg(0, std::ios::beg);

    out = new uint8_t[4 + name_length + file_size]; // Allocate memory for the packed file

    std::memcpy(out, &file_size, 4); // Copy four bytes of file size to output
    const char * r_filename = remote_filename.data();
    std::memcpy(out + 4, r_filename, name_length); // Copy the file name in
    stream_local.read((char *) (out + 4 + name_length), file_size); // Copy the file contents in

    stream_local.close(); // Close the opened file stream

    *out_size = 4 + name_length + file_size; // Output the size of the result

    return out; // Return the packed file. Caller is responsible for free()ing this
}

/**
 * @brief Public method to upload a local file to the camera.
 * 
 * @param[in] local_filename The local path and filename to send
 * @param[in] remote_filename The path and filename to store the file on the camera
 * @param[in] timeout (optional) The timeout for each PTP call
 * @return True on success
 */
bool CHDKCamera::upload_file(const std::string local_filename, const std::string remote_filename, const int timeout)
{
    uint8_t * packed;
    uint32_t packed_size;
    PTPContainer cmd(PTPContainer::CONTAINER_TYPE_COMMAND, 0x9999);
    PTPContainer data(PTPContainer::CONTAINER_TYPE_DATA, 0x9999);
    PTPContainer resp, out_data;

    packed = CHDKCamera::_pack_file_for_upload(&packed_size, local_filename, remote_filename);

    cmd.add_param(PTP_CHDK_UploadFile);
    data.set_payload(packed, packed_size);

    this->ptp_transaction(cmd, data, false, resp, out_data);

    delete[] packed;

    return (resp.get_param_n(0) == CHDK_PTP_RC_OK);
}

} /* namespace PTP */
