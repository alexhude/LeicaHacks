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

#ifndef LIBEASYPTP_PTPUSB_H_
#define LIBEASYPTP_PTPUSB_H_

#ifdef __FreeBSD__
#include <libusb.h>
#else
#include <libusb-1.0/libusb.h>
#endif

#include "libeasyptp/IPTPComm.hpp"

namespace EasyPTP
{

class PTPUSB : public IPTPComm
{
private:
    struct libusb_context *context;
    libusb_device_handle *handle;
    int usb_error;
    struct libusb_interface_descriptor intf;
    uint8_t ep_in;
    uint8_t ep_out;

    bool open(libusb_device * dev);
    libusb_device * find_first_camera();
    void init();

	static const int INTERFACE_CLASS_PTP = 6;

    bool isPTPDevice(libusb_device *device);
    bool isPTPInterface(struct libusb_interface interface);
    void getPTPInterface(libusb_device *dev, struct libusb_interface_descriptor & intf, libusb_device_handle *& handle);
    void getEndpoints(struct libusb_interface_descriptor *intf, uint8_t &ep_in, uint8_t &ep_out);
	bool isBulkInEndpoint(const struct libusb_endpoint_descriptor* endpoint);
	bool isOutEndpoint(const struct libusb_endpoint_descriptor* endpoint);

    class USBConfigDescriptor
    {
    private:
    	struct libusb_config_descriptor * desc;
    public:
    	USBConfigDescriptor(libusb_device *device);
    	~USBConfigDescriptor();
    	struct libusb_config_descriptor *get();
    };

public:
    PTPUSB();
    PTPUSB(libusb_device * dev);
    ~PTPUSB();
    void connect_to_first();
    void connect_to_serial_no(std::string serial);
    virtual bool _bulk_write(const unsigned char * bytestr, const int length, const int timeout);
    virtual bool _bulk_read(unsigned char * data_out, const int size, int * transferred, const int timeout);
    virtual bool is_open();
    void close();
};

}

#endif /* LIBPTP_PP_PTPUSB_H_ */
