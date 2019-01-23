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
 * @file LVData.cpp
 * 
 * @brief Small data structure for handling Live View data
 * 
 * LVData is responsible for handling live view data and providing convenience
 * functions for accessing the data.  Functions from CHDKCamera will return an
 * LVData object, which can be manipulated to retrieve data to display an image.
 */

#include <cstring>
#include <stdint.h>

#include "libeasyptp/PTPErrors.hpp"
#include "libeasyptp/LVData.hpp"
#include "libeasyptp/PTPContainer.hpp"

namespace EasyPTP
{

/**
 * @brief Initialize a blank live view data container
 */
LVData::LVData() : LVData(NULL, 0)
{

}

/**
 * @brief Initialize a live view data container with the data from \a payload
 *
 * @param[in] payload The address of the first byte of a PTP payload
 * @param[in] payload_size The number of bytes in \a payload
 * @see LVData::read
 */
LVData::LVData(const uint8_t * payload, const int payload_size) :
vp_head(new lv_data_header), fb_desc(new lv_framebuffer_desc), payload(NULL)
{
    if (payload != NULL)
    {
        this->read(payload, payload_size);
    }
}

/**
 * @brief Frees up memory malloc()ed by \c LVData
 */
LVData::~LVData()
{
    delete this->vp_head;
    delete this->fb_desc;
    delete this->payload;
}

/**
 * @brief Read data from \a payload into the live view data structures
 *
 * Parases \a payload for the necessary parts of the payload and places them
 * in our internal structures.  Also stores a copy of the complete payload
 * for later use in retrieving data.  This way, we only spend CPU time on the
 * data retrieval we NEED to make.
 *
 * @param[in] payload The address of the first byte of a PTP payload
 * @param[in] payload_size The number of bytes in the payload
 * @exception LVDATA_NOT_ENOUGH_DATA If payload_size given cannot possibly be large
 *              enough to actually contain live view data.
 * @todo The casting here is pretty bad. How can I clean this up?
 */
void LVData::read(const uint8_t * payload, const unsigned int payload_size)
{
    if (payload_size < (sizeof (lv_data_header) + sizeof (lv_framebuffer_desc)))
    {
        throw ERR_LVDATA_NOT_ENOUGH_DATA;
    }

    if (this->payload != NULL)
    {
        delete[] this->payload; // Free up the payload if we're overwriting this object
        this->payload = NULL;
    }

    this->payload = new uint8_t[payload_size];

    std::memcpy(this->payload, payload, payload_size); // Copy the payload we're reading in into OUR payload

    // Parse the payload data into vp_head and fb_desc
    std::memcpy(this->vp_head, this->payload, sizeof (lv_data_header));
    std::memcpy(this->fb_desc, this->payload + this->vp_head->vp_desc_start, sizeof (lv_framebuffer_desc));
}

/**
 * @brief Read live view data directly from a \c PTPContainer
 *
 * This function exists so that we can hide the actual payload data from
 * calling functions, and just pass \c PTPContainer s around.  In a
 * perfect data-hiding world, \c PTPContainer and \c LVData might be friend
 * classes, so that methods to access the payload could be kept protected.
 *
 * @param[in] container The \c PTPContainer to read live view data from
 * @see LVData::read(uint8_t * payload, int payload_size)
 */
void LVData::read(const PTPContainer& container)
{
    int payload_size;
    unsigned char * payload;

    payload = container.get_payload(&payload_size);

    this->read(payload, payload_size);

    delete[] payload;
}

/**
 * @brief Get live view data in RGB format
 *
 * This function is quite a doozy, and likely to be the most CPU-intense task
 * an \c LVData can perform.  This method takes the live view data (in YUV format)
 * from the payload and converts it to RGB so that it can actually be used.  The
 * \a skip parameter will depend on which camera is used.  Size, width, and height
 * are calculated from properties of the live view data, to hide the underlying structure.
 *
 * @warning This function malloc()s space for the resulting data. Be sure to free() it!
 *
 * @param[out] out_size The size of the resulting RGB data
 * @param[out] out_width The width of the resulting RGB image
 * @param[out] out_height The height of the resulting RGB image
 * @param[in]  skip If true, skips two pixels of every four (required on some cameras)
 * @return The address of the first byte of the resulting RGB image
 * @see http://chdk.wikia.com/wiki/Frame_buffers#Viewport, http://trac.assembla.com/chdk/browser/trunk/tools/yuvconvert.c
 */
uint8_t * LVData::get_rgb(int * out_size, int * out_width, int * out_height, const bool skip) const
{
    uint8_t * vp_data;
    int vp_size;
    vp_size = (this->fb_desc->buffer_width * this->fb_desc->visible_height * 12) / 8; // 12 bpp
    vp_data = new uint8_t[vp_size]; // Allocate memory for YUV data 
    std::memcpy(vp_data, this->payload + this->fb_desc->data_start, vp_size); // Copy YUV data into vp_data

    int par = skip ? 2 : 1; // If skip, par = 2 ; else, par = 1

    *out_width = this->fb_desc->visible_width / par; // Vertical width of output
    unsigned int dispsize = *out_width * (this->fb_desc->visible_height); // Size of output
    *out_size = dispsize * 3; // RGB output size

    uint8_t * out = new uint8_t[*out_size]; // Allocate space for RGB output

    uint8_t * prgb_data = out; // Pointer we can manipulate to transverse RGB output memory
    uint8_t * p_yuv = vp_data; // Pointer we can manipulate to transverse YUV input memory

    int i;
    // Transverse input and output. For each four RGB pixels, we increment 6 YUV bytes
    //  See: http://chdk.wikia.com/wiki/Frame_buffers#Viewport
    // This magical code borrowed from http://trac.assembla.com/chdk/browser/trunk/tools/yuvconvert.c
    for (i = 0; i < (this->fb_desc->buffer_width * this->fb_desc->visible_height); i += 4, p_yuv += 6)
    {
        this->yuv_to_rgb(&prgb_data, p_yuv[1], p_yuv[0], p_yuv[2]);
        this->yuv_to_rgb(&prgb_data, p_yuv[3], p_yuv[0], p_yuv[2]);

        if (skip) continue; // If we skip two, go to the next iteration

        this->yuv_to_rgb(&prgb_data, p_yuv[4], p_yuv[0], p_yuv[2]);
        this->yuv_to_rgb(&prgb_data, p_yuv[5], p_yuv[0], p_yuv[2]);
    }

    *out_height = this->fb_desc->visible_height;

    delete[] vp_data;
    ; // We don't need this anymore

    return out; // It's up to the caller to free() this when done
}

/**
 * @brief A helper function to clip an int to a uint8_t
 *
 * @param[in] v The integer to clip
 * @return A 8-bit unsigned integer between 0 and 255
 */
uint8_t LVData::clip(const int v)
{
    if (v < 0) return 0;
    if (v > 255) return 255;
    return v;
}

/**
 * @brief Convert a YUV triplet to RGB values.
 *
 * @note This modifies the \a dest pointer passed in.
 * @param[in] dest A pointer to the address of the first byte of the resulting data
 * @param[in] y The Y value to convert
 * @param[in] u The U value to convert
 * @param[in] v The V value to convert
 * @see http://www.fourcc.org/fccyvrgb.php
 */
void LVData::yuv_to_rgb(uint8_t **dest, const uint8_t y, const int8_t u, const int8_t v)
{
    /*
     *((*dest)++) = LVData::clip(((y<<12) +          v*5743 + 2048)>>12);
     *((*dest)++) = LVData::clip(((y<<12) - u*1411 - v*2925 + 2048)>>12);
     *((*dest)++) = LVData::clip(((y<<12) + u*7258          + 2048)>>12);
     */
    // Testing alternative formula:
    *((*dest)++) = LVData::clip(y + 1.402 * v);
    *((*dest)++) = LVData::clip(y - 0.34414 * u - 0.71414 * v);
    *((*dest)++) = LVData::clip(y + 1.772 * u);
}

/**
 * @brief Retrieve the live view version from the header data
 *
 * @note Expects the minor version number to be one digit.
 *
 * @return The version of this live view data
 */
float LVData::get_lv_version() const
{
    if (this->vp_head == NULL) return -1;

    return this->vp_head->version_major + this->vp_head->version_minor / 10.0;
}

} /* namespace PTP */
