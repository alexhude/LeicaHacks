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

#ifndef LIBEASYPTP_PTPERRORS_H_
#define	LIBEASYPTP_PTPERRORS_H_

namespace EasyPTP
{

enum LIBPTP_PP_ERRORS
{
    ERR_NONE = 0,
    ERR_CANNOT_CONNECT,
    ERR_NO_DEVICE,
    ERR_ALREADY_OPEN,
    ERR_NOT_OPEN,
    ERR_CANNOT_RECV,
    ERR_TIMEOUT,
    ERR_INVALID_RESPONSE,
    ERR_NOT_IMPLEMENTED,
    ERR_USB_ERROR,
    ERR_NO_PTP_INTERFACE,

    ERR_PTPCONTAINER_NO_PAYLOAD,
    ERR_PTPCONTAINER_INVALID_PARAM,

    ERR_LVDATA_NOT_ENOUGH_DATA,
};
}

#endif	/* LIBEASYPTP_PTPERRORS_H_ */
