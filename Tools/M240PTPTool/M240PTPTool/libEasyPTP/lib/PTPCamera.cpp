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
 * @file PTPCamera.cpp
 * 
 * @brief A placeholder for generic PTP cameras
 * 
 * Eventually, this file will be full of functions to ease communication with
 * standard (non-CHDK) PTP cameras.
 */

#include "libeasyptp/PTPErrors.hpp"
#include "libeasyptp/PTPCamera.hpp"

namespace EasyPTP
{

/**
 * @brief Creates an empty \c PTPCamera.
 *
 * @warning This class is not yet implemented. Creating an object of type
 *          \c PTPCamera will only result in a warning printed to \c stderr.
 */
PTPCamera::PTPCamera()
{
    throw ERR_NOT_IMPLEMENTED;
}

} /* namespace PTP */
