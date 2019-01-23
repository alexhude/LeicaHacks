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

#ifndef LIBEASYPTP_LVDATA_H_
#define LIBEASYPTP_LVDATA_H_

namespace EasyPTP
{
#include "libeasyptp/chdk/live_view.h"

class PTPContainer; // Forward delcaration for this is enough

class LVData
{
private:
    lv_data_header * vp_head;
    lv_framebuffer_desc * fb_desc;
    uint8_t * payload;
    static uint8_t clip(const int v);
    static void yuv_to_rgb(uint8_t **dest, const uint8_t y, const int8_t u, const int8_t v);

public:
    LVData();
    LVData(const uint8_t * payload, const int payload_size);
    ~LVData();
    void read(const uint8_t * payload, const unsigned int payload_size);
    void read(const PTPContainer& container); // Could this make life easier?
    uint8_t * get_rgb(int * out_size, int * out_width, int * out_height, const bool skip = false) const; // Some cameras don't require skip
    float get_lv_version() const;
};

}

#endif /* LIBEASYPTP_LVDATA_H_ */
