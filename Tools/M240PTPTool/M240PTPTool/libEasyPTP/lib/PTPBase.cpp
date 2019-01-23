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
 * @file PTPBase.cpp
 * 
 * @brief The base functionality that PTP communication is built on.
 * 
 * This file contains the PTPBase class, from which PTPCamera and CHDKCamera
 * are extended.  PTPBase is designed to handle all communication with libusb
 * and with setting up communication with the camera, so that the Camera classes
 * can just talk to the camera using the correct protocol.
 */

#include <cstring>
#include <stdint.h>

#include "libeasyptp/PTPErrors.hpp"
#include "libeasyptp/PTPBase.hpp"
#include "libeasyptp/PTPContainer.hpp"
#include "libeasyptp/IPTPComm.hpp"

namespace EasyPTP
{

/**
 * Creates a new, empty \c PTPBase object.  Can then call
 * \c PTPBase::open to connect to a camera.
 */
PTPBase::PTPBase() : PTPBase(NULL)
{

}

/**
 * Creates a new \c PTPBase object, using \c comm for the communication
 * protocol class.
 */
PTPBase::PTPBase(IPTPComm * protocol) :
protocol(NULL), _transaction_id(0)
{
    // If protocol == NULL, this will just re-set protocol to NULL, which is fine
    this->set_protocol(protocol);
}

/**
 * Destructor for a \c PTPBase object.  If connected to a camera, this
 * will release the interface, and close the handle.
 */
PTPBase::~PTPBase()
{

}

void PTPBase::set_protocol(IPTPComm * protocol)
{
    this->protocol = protocol;
}

/**
 * Send the data contained in \a cmd to the connected camera.
 *
 * @param[in] cmd The \c PTPContainer containing the command/data to send.
 * @param[in] timeout The maximum number of seconds to attempt to send for.
 * @return 0 on success, libusb error code otherwise.
 * @see PTPBase::_bulk_write, PTPBase::recv_ptp_message
 */
int PTPBase::send_ptp_message(const PTPContainer& cmd, const int timeout)
{
    if (this->protocol == NULL || this->protocol->is_open() == false)
    {
        throw ERR_NOT_OPEN;
        return -1;
    }

    unsigned char * packed = cmd.pack();
    int ret = this->protocol->_bulk_write(packed, cmd.get_length(), timeout);
    delete[] packed;

    return ret;
}

/**
 * @brief Recives a \c PTPContainer from the camera and returns it.
 *
 * This function works by first reading in a buffer of 512 bytes from the camera
 * to determine the length of the PTP message it will receive.  If necessary, it
 * then makes another \c PTPBase::_bulk_read call to read in the rest of the
 * data.  Finally, \c PTPContainer::unpack is called to place the data in \a out.
 *
 * @warning \a timeout is passed to each call to \c PTPBase::_bulk_read.  Therefore,
 *          this function could take up to 2 * \a timeout seconds to return.
 *
 * @param[out] out A pointer to a PTPContainer that will store the read PTP message.
 * @param[in]  timeout The maximum number of seconds to wait to read each time.
 * @see PTPBase::_bulk_read, PTPBase::send_ptp_message
 */
void PTPBase::recv_ptp_message(PTPContainer& out, const int timeout)
{
    if (this->protocol == NULL || this->protocol->is_open() == false)
    {
        throw ERR_NOT_OPEN;
        return;
    }

    // Determine size we need to read
    unsigned char * buffer = new unsigned char[512];
    int read = 0;
    this->protocol->_bulk_read(buffer, 512, &read, timeout); // TODO: Error checking on response
    uint32_t size = 0;
    if (read < 4)
    {
        // If we actually read less than four bytes, we can't copy four bytes out of the buffer.
        // Also, something went very, very wrong
        throw ERR_CANNOT_RECV;
        return;
    }
    std::memcpy(&size, buffer, 4); // The first four bytes of the buffer are the size

    // Copy our first part into the output buffer -- so we can reuse buffer
    unsigned char * out_buf = new unsigned char[size];
    if (size < 512)
    {
        std::memcpy(out_buf, buffer, size);
    }
    else
    {
        std::memcpy(out_buf, buffer, 512);
        // We've already read 512 bytes... read the rest!
        this->protocol->_bulk_read(&out_buf[512], size - 512, &read, timeout);
    }

    out.unpack(out_buf);

    delete[] out_buf;
    delete[] buffer;
}

/**
 * @brief Perform a complete write, and optionally read, PTP transaction.
 * 
 * At minimum, it is required that \a cmd is not \c NULL.  All other containers
 * are checked for NULL values before reading/writing.  Note that this function
 * will also modify \a cmd and \a data to place a generated transaction ID in them,
 * required by the PTP protocol.
 *
 * Although not enforced by this function, \a cmd should be a \c PTPContainer containing
 * a command, and \a data (if given) should be a \c PTPContainer containing data.
 *
 * Note that even if \a receiving is false, PTP requires that we receive a response.
 * If provided, \a out_resp will be populated with the command response, even if
 * \a receiving is false.
 *
 * @warning \c PTPBase::_bulk_read and \c PTPBase::_bulk_write are called multiple
 *          times during the execution of this function, and \a timeout is passed to each
 *          of them individually.  Therefore, this function could take much more than
 *          \a timeout seconds to return.
 *
 * @param[in]  cmd       A \c PTPContainer containing the command to send to the camera.
 * @param[in]  data      (optional) A \c PTPContainer containing the data to be sent with the command.
 * @param[in]  receiving Whether or not to receive data in addition to a response from the camera.
 * @param[out] out_resp  (optional) A \c PTPContainer where the camera's response will be placed.
 * @param[out] out_data  (optional) A \c PTPContainer where the camera's data response will be placed.
 * @param[in]  timeout   The maximum number of seconds each \c PTPBase::_bulk_read or \c PTPBase::_bulk_write
 *                       should attempt to communicate for.
 * @see PTPBase::send_ptp_message, PTPBase::recv_ptp_message
 */
void PTPBase::ptp_transaction(PTPContainer& cmd, PTPContainer& data, const bool receiving, PTPContainer& out_resp, PTPContainer& out_data, const int timeout)
{
	// TODO: Use received data
//    bool received_data = false;
    bool received_resp = false;

    cmd.transaction_id = this->get_and_increment_transaction_id();
    this->send_ptp_message(cmd, timeout);

    if (!data.is_empty())
    {
        // Only send data if it doesn't have an empty payload
        data.transaction_id = cmd.transaction_id;
        this->send_ptp_message(data, timeout);
    }

    if (receiving)
    {
        PTPContainer out;
        this->recv_ptp_message(out, timeout);
        if (out.type == PTPContainer::CONTAINER_TYPE_DATA)
        {
//            received_data = true;
            // TODO: It occurs to me that pack() and unpack() might be inefficient. Let's try to find a better way to do this.
            unsigned char * packed = out.pack();
            out_data.unpack(packed);
            delete[] packed;
        }
        else if (out.type == PTPContainer::CONTAINER_TYPE_RESPONSE)
        {
            received_resp = true;
            unsigned char * packed = out.pack();
            out_resp.unpack(packed);
            delete[] packed;
        }
    }

    if (!received_resp)
    {
        // Read it anyway!
        // TODO: We should return response AND data...
        this->recv_ptp_message(out_resp, timeout);
    }
}

/**
 * @brief Retrieves our current transaction ID and increments it
 *
 * @return The current transaction id (starting at 0)
 * @see PTPBase::ptp_transaction
 */
int PTPBase::get_and_increment_transaction_id()
{
    uint32_t ret = this->_transaction_id;
    this->_transaction_id = this->_transaction_id + 1;
    return ret;
}

} /* namespace PTP */
