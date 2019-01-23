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

#ifndef LIBEASYPTP_IPTPCOMM_H_
#define LIBEASYPTP_IPTPCOMM_H_

namespace EasyPTP
{

/**
 * @class IPTPComm
 * @brief An interface containing basic methods for writing and reading PTP data
 * 
 * With \c IPTPComm, it's possible to extend libptp++ to communicate over 
 * essentially any protocol.  This class serves as an interface for the 
 * underlying implementation.  You can then pass a class that implements 
 * \c IPTPComm to \c PTPCamera or \c CHDKCamera and change the underlying 
 * protocol.
 * 
 * Depending on the protocol, you'll probably need some additional methods to do
 * some communication setup.  Note that \c PTPBase will only use this to send
 * and receive data, and not perform any of the setup for you.  You *must* set
 * up the connection on your own before passing a communication protocol to
 * \c PTPBase, or something could fail.
 * 
 * @todo I'd like to ship libptp++ with at least a libusb(x) implementaiton of
 *       IPTPComm
 * @todo Socket implementation of IPTPComm?
 * @todo Common exceptions that can be used in any implementation?
 */
class IPTPComm
{
public:

    virtual ~IPTPComm()
    {
    }
    /**
     * @brief Check that we have open communication
     * 
     * @return True if we can _bulk_write and _bulk_read, false otherwise
     */
    virtual bool is_open() = 0;
    /**
     * @brief Write data to the protocol
     * 
     * _bulk_write handles writing data to the protocol.  The first 
     * parameter is the data that needs to be written, followed by the 
     * length of that data, and optionally a timeout parameter.  
     * 
     * @return true if the data was successfully written, or false
     * if there was a problem
     * @todo Common exceptions
     */
    virtual bool _bulk_write(const unsigned char * bytestr, const int length, const int timeout = 0) = 0;
    /**
     * @brief Read data from the protocol
     * 
     * _bulk_read handles reading data from the protocol.  It must return
     * this data as a pointer to an unsigned character (allocated via
     * new unsigned char[], so that it can be delete[]-ed later).  Size is 
     * the amount of data we would like to read, and transferred is how much
     * gets actually transferred (it is possible to have transferred < size).
     * Optionally, a timeout can also be provided.
     * 
     * @return true if the data was read successfully, false if there was a
     * problem
     * @todo Common exceptions
     */
    virtual bool _bulk_read(unsigned char * data_out, const int size, int * transferred, const int timeout = 0) = 0;
};

}

#endif /* LIBEASYPTP_IPTPCOMM_H_ */
