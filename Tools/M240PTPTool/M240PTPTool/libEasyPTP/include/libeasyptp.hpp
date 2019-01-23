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
 * @file libeasyptp.hpp
 * 
 * @brief A conversion of pyptp2 and all that comes with it to C++
 * 
 * libptp2 is nice, but appears to be tightly bound to ptpcam.  There
 * are a few other CHDK-specific programs to communicate with a camera
 * through PTP, but all contain source code that is tightly integrated
 * and difficult to read.
 * 
 * While this library should be able to communicate with any PTP camera
 * through the \c PTPCamera interface, it's primary purpose is to allow
 * easy communication with cameras running CHDK through \c CHDKCamera.
 * 
 * This library has two goals:
 *  -# Provide all functionality of pyptp2 through a C++ interface.
 *  -# Be easy to use, and well-documented.
 *  
 * @author Bobby Graese <bobby.graese@gmail.com>
 * 
 * @see http://code.google.com/p/pyptp2/
 * @see http://libptp.sourceforge.net/
 * 
 * @version 0.1
 */

/**
 * \mainpage libeasyptp API Reference
 * 
 * \section intro Introduction
 *
 * libeasyptp is an open-source C++ library for communicating with PTP devices in
 * the easiest way possible.  It is a port and extension of the pyptp2 API to
 * C++.  This API was chosen to avoid designing an API from the ground up, and
 * because it seems to be fairly stable and useful.  However, the pyptp2 library
 * was able to use some Python conveniences that aren't available in C++, so
 * some additions have been made to this API.
 *
 * This library does not assume that the developer knows anyting about PTP, or
 * how it handles its transactions over USB.  Instead, all these functions are
 * abstracted out to library functions, and these library functions attempt to
 * hide the underlying USB interface as much as possible.  In some cases, it is
 * simply not feasibly to hide this interface, so it is exposed to the
 * developer.
 *
 * \section gettingstarted Getting Started
 *
 * If you learn best by reading documentation, head over to the "Classes" page,
 * and you can read an overview of what classes are available, as well as what
 * functionality each of their methods provide.
 *
 * However, most likely, the "Examples" page will be more useful.  This library
 * is designed so that a lot can be accomplished in as few calls as possible, so
 * the examples should help get you a quick start to using libeasyptp.
 */

/**
 * \page examples Examples
 *
 * Note: this page is mostly incomplete. More examples coming in the future.
 *
 * \section simple A Simple Example
 *
 * This example simply finds the first PTP camera available, connects to it,
 * and asks CHDK to put the camera in "record" mode.
\code
libusb_init(NULL);  // Make sure to initialize libusb first!

libusb_device * dev = CHDKCamera::find_first_camera();

CHDKCamera cam; 
cam.open(dev);

// Execute a lua script to switch the camera to "Record" mode.
//  Second parameter, error_code, is NULL, because we don't care if an error
//  occurs, and we aren't blocking to wait for one.
cam.execute_lua("switch_mode_usb(1)", NULL);

// The camera is closed automatically when the cam object is destroyed

// Be sure to exit libusb
libusb_exit(NULL);
\endcode
 *
 */

#ifndef LIBEASYPTP_H_
#define LIBEASYPTP_H_

#include <libusb-1.0/libusb.h>

// This serves as a global "include" file -- include this to grab all the other
//  headers, too
#include "libeasyptp/PTPBase.hpp"
#include "libeasyptp/CHDKCamera.hpp"
#include "libeasyptp/LVData.hpp"
#include "libeasyptp/PTPCamera.hpp"
#include "libeasyptp/PTPContainer.hpp"
#include "libeasyptp/IPTPComm.hpp"
#include "libeasyptp/PTPUSB.hpp"

namespace EasyPTP
{

// Force these definitions into the PTP namespace
#include "libeasyptp/chdk/live_view.h"
#include "libeasyptp/chdk/ptp.h"

}

#endif /* LIBEASYPTP_H_ */
