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

#ifndef LIBEASYPTP_PTPCONTAINER_H_
#define LIBEASYPTP_PTPCONTAINER_H_

namespace EasyPTP
{

class PTPContainer
{
private:
    static const uint32_t default_length = sizeof (uint32_t) + sizeof (uint32_t) + sizeof (uint16_t) + sizeof (uint16_t);
    uint32_t length;
    unsigned char * payload; // We'll deal with this completely internally
public:

    enum CONTAINER_TYPE
    {
        CONTAINER_TYPE_COMMAND = 1,
        CONTAINER_TYPE_DATA = 2,
        CONTAINER_TYPE_RESPONSE = 3,
        CONTAINER_TYPE_EVENT = 4
    };

    uint16_t type;
    uint16_t code;
    uint32_t transaction_id; // We'll end up setting this externally
    PTPContainer();
    PTPContainer(const uint16_t type, const uint16_t op_code);
    PTPContainer(const unsigned char * data);
    ~PTPContainer();
    void add_param(const uint32_t param);
    void set_payload(const void * payload, const int payload_length);
    unsigned char * pack() const;
    unsigned char * get_payload(int * size_out) const; // This might end up being useful...
    uint32_t get_length() const; // So we can get, but not set
    void unpack(const unsigned char * data);
    uint32_t get_param_n(const uint32_t n) const;
    bool is_empty() const;
};

}

#endif /* LIBEASYPTP_PTPCONTAINER_H_ */
