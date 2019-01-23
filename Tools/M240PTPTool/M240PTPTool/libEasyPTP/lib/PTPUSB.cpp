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

#include <string>
#include <vector>
#ifdef __FreeBSD__
#include <libusb.h>
#else
#include <libusb-1.0/libusb.h>
#endif

#include "libeasyptp/PTPErrors.hpp"
#include "libeasyptp/PTPUSB.hpp"

namespace EasyPTP
{
PTPUSB::PTPUSB() : PTPUSB(NULL)
{
}

PTPUSB::PTPUSB(libusb_device * dev) : handle(NULL), usb_error(0),
ep_in(0), ep_out(0)
{
	libusb_init(&context);

    // If we were passsed a device, open it!
    if (dev != NULL)
    {
        this->open(dev);
    }
}

PTPUSB::~PTPUSB()
{
    this->close();

	// Be sure to exit libusb
	libusb_exit(context);
}

void PTPUSB::connect_to_first()
{
    // Find the first camera
    libusb_device * dev = this->find_first_camera();
    if (dev == NULL) // TODO: Throw exception
        return;

    this->open(dev);
}

/**
 * @brief Find the first camera which is connected.
 *
 * Asks libusb for all the devices connected to the computer, and returns
 * the first PTP device it can find.  
 * 
 * @return A pointer to a \c libusb_device which represents the camera found, or NULL if none found.
 */
libusb_device * PTPUSB::find_first_camera()
{
    libusb_device **usb_device_list;
    libusb_device *ptp_device = NULL;
    ssize_t device_count = libusb_get_device_list(context, &usb_device_list);
    if (device_count < 0)
        return NULL;

    for (int i = 0; i < device_count; i++)
    {
        if(isPTPDevice(usb_device_list[i]))
        {
            ptp_device = usb_device_list[i];
            libusb_ref_device(ptp_device);
            break;
        }
    }

    libusb_free_device_list(usb_device_list, 1); // Free the device list with dereferencing. Shouldn't delete our device, since we ref'd it

    return ptp_device;
}

/**
 * @brief Checks if the given device is a PTP device
 * 
 * @param device The libusb_device to check
 * @return bool
 */
bool PTPUSB::isPTPDevice(libusb_device *device)
{
    USBConfigDescriptor desc(device);

    for (int j = 0; j < desc.get()->bNumInterfaces; j++)
    {
        if(isPTPInterface(desc.get()->interface[j]))
        	return true;
    }

    return false;
}

bool PTPUSB::isPTPInterface(struct libusb_interface interface)
{
	for (int k = 0; k < interface.num_altsetting; k++)
	{
		if (interface.altsetting[k].bInterfaceClass == INTERFACE_CLASS_PTP)
			return true;
	}

	return false;
}

/**
 * @brief Opens the camera specified by \a dev.
 *
 * The device must already be referenced once.  Calling this function will unref
 * the device once.  It's recommended you use a PTPUSB function to find the USB
 * device to open, but that isn't a strict requirement.
 *
 * @param[in] dev The \c libusb_device which specifies which device to connect to.
 * @exception PTP::ERR_ALREADY_OPEN if this \c PTPUSB already has an open device.
 * @exception PTP::ERR_CANNOT_CONNECT if we cannot connect to the camera specified.
 * @return true if we successfully connect, false otherwise.
 */
bool PTPUSB::open(libusb_device * dev)
{
    if (is_open()) // Handle will be non-null if the device is already open
        throw ERR_ALREADY_OPEN;

    int err = libusb_open(dev, &(this->handle)); // Open the device, placing the handle in this->handle
    if (err != LIBUSB_SUCCESS)
        throw ERR_CANNOT_CONNECT;

    libusb_unref_device(dev); // We needed this device refed before we opened it, so we added an extra ref. open adds another ref, so remove one ref

    getPTPInterface(dev, this->intf, this->handle);

    getEndpoints(&this->intf, this->ep_in, this->ep_out);

    // If we haven't detected an error by now, assume that this worked.
    return true;
}

bool PTPUSB::isBulkInEndpoint(const struct libusb_endpoint_descriptor* endpoint)
{
	return (((endpoint->bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_IN)
			&& ((endpoint->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) == LIBUSB_TRANSFER_TYPE_BULK));
}

bool PTPUSB::isOutEndpoint(const struct libusb_endpoint_descriptor* endpoint)
{
	return ((endpoint->bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_OUT);
}

void PTPUSB::getEndpoints(struct libusb_interface_descriptor *intf, uint8_t &ep_in, uint8_t &ep_out)
{
	bool found_ep_in = false, found_ep_out = false;
	for (int i = 0; i < intf->bNumEndpoints; i++)
	{
		const struct libusb_endpoint_descriptor * endpoint = &(intf->endpoint[i]);
		if (isBulkInEndpoint(endpoint))
		{
			ep_in = endpoint->bEndpointAddress;
			found_ep_in = true;
		}
		else if (isOutEndpoint(endpoint))
		{
			ep_out = endpoint->bEndpointAddress;
			found_ep_out = true;
		}
		if(found_ep_in && found_ep_out) return;
	}
	throw ERR_NO_PTP_INTERFACE;
}

/**
 * Claim a PTP interface from the device dev, store the descriptor and
 * handle in intf and handle respectively.
 *
 * @TODO Merge some code with isPTPDevice/isPTPInterface.  A lot of
 * 		 that code is duplicated here currently.
 */
void PTPUSB::getPTPInterface(libusb_device *dev, struct libusb_interface_descriptor & intf, libusb_device_handle *& handle)
{
	USBConfigDescriptor* desc = new USBConfigDescriptor(dev);

	for (int i = 0; i < desc->get()->bNumInterfaces; i++)
	{
		struct libusb_interface interface = desc->get()->interface[i];
		for (int j = 0; j < interface.num_altsetting; j++)
		{
			struct libusb_interface_descriptor altsetting = interface.altsetting[j];
			if (altsetting.bInterfaceClass == INTERFACE_CLASS_PTP)
			{ // If this has the PTP interface
				intf = altsetting;
				libusb_claim_interface(handle, altsetting.bInterfaceNumber); // Claim the interface -- Needs to be done before I/O operations
				return;
			}
		}
	}

	throw ERR_NO_PTP_INTERFACE;
}

/**
 * Perform a \c libusb_bulk_transfer to the "out" endpoint of the connected camera.
 *
 * @warning Make sure \a bytestr is at least \a length bytes in length.
 * @param[in] bytestr Bytes to write through USB.
 * @param[in] length  Number of bytes to read from \a bytestr.
 * @param[in] timeout The maximum number of seconds to attempt to send for.
 * @return 0 on success, libusb error code otherwise.
 * @exception PTP::ERR_NOT_OPEN if not connected to a camera.
 * @see PTPUSB::_bulk_read
 */
bool PTPUSB::_bulk_write(const unsigned char * bytestr, const int length, const int timeout)
{
    int transferred;

    if (!is_open())
        throw EasyPTP::ERR_NOT_OPEN;

    std::vector<unsigned char> write_data;
    write_data.resize(length);
    std::copy(bytestr, bytestr + length, write_data.data());

    // TODO: Return the amount of data transferred? Check it here? What should we do if not enough was sent?
    bool ret = (libusb_bulk_transfer(this->handle, this->ep_out, write_data.data(), length, &transferred, timeout) == 0);

    return ret;
}

/**
 * Perform a \c libusb_bulk_transfer to the "in" endpoint of the connected camera.
 *
 * @warning Make sure \a data_out has enough memory allocated to read at least \a size bytes.
 * @param[out] data_out    The data read from the camera.
 * @param[in]  size        The number of bytes to attempt to read.
 * @param[out] transferred The number of bytes actually read.
 * @param[in]  timeout     The maximum number of seconds to attempt to read for.
 * @return 0 on success, libusb error code otherwise.
 * @exception PTP::ERR_NOT_OPEN if not connected to a camera.
 * @see PTPUSB::_bulk_read
 */
bool PTPUSB::_bulk_read(unsigned char * data_out, const int size, int * transferred, const int timeout)
{
    if (!is_open())
        throw ERR_NOT_OPEN;

    // TODO: Return the amount of data transferred? We might get less than we ask for, which means we need to tell the calling function?
    return libusb_bulk_transfer(this->handle, this->ep_in, data_out, size, transferred, timeout) == 0;
}

/**
 * @brief Returns true if we can _bulk_read and _bulk_write
 */
bool PTPUSB::is_open()
{
    // This should work... and be fairly straightforward!
    return (this->handle != NULL);
}

/**
 * Closes the opened USB object.
 * @todo Check for errors in the calls
 */
void PTPUSB::close()
{
    if (is_open())
    {
        libusb_release_interface(this->handle, this->intf.bInterfaceNumber);
        libusb_close(this->handle);
        this->handle = NULL;
    }
}

/**
 * The USBConfigDescriptor class is used as a helper to ensure that cleanup
 * for the descriptor happens.
 */
PTPUSB::USBConfigDescriptor::USBConfigDescriptor(libusb_device *device)
{
	if (libusb_get_active_config_descriptor(device, &desc) != LIBUSB_SUCCESS)
		throw ERR_USB_ERROR;
}

PTPUSB::USBConfigDescriptor::~USBConfigDescriptor()
{
	libusb_free_config_descriptor(desc);
}

struct libusb_config_descriptor * PTPUSB::USBConfigDescriptor::get()
{
	return desc;
}

}
