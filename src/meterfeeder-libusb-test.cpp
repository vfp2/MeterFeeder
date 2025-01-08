#include <iostream>
#include <libftdi1/ftdi.h>
#include <libusb-1.0/libusb.h>  // Include libusb for descriptor functions

int main() {
    struct ftdi_context ftdi;
    ftdi_device_list *dev_list = nullptr;
    int res;

    // Initialize libftdi context
    res = ftdi_init(&ftdi);
    if (res < 0) {
        std::cerr << "Error initializing libftdi" << std::endl;
        return 1;
    }

    // Query all FTDI devices
    res = ftdi_usb_find_all(&ftdi, &dev_list, 0, 0);  // 0 for any vendor/product ID
    if (res < 0) {
        std::cerr << "Error finding FTDI devices" << std::endl;
        ftdi_deinit(&ftdi);
        return 1;
    }

    // Print details similar to dmesg output
    ftdi_device_list *current_device = dev_list;
    while (current_device != nullptr) {
        struct libusb_device_descriptor descriptor;

        // Use libusb_get_device_descriptor to get the device descriptor
        res = libusb_get_device_descriptor(current_device->dev, &descriptor);
        if (res < 0) {
            std::cerr << "Error getting device descriptor" << std::endl;
            current_device = current_device->next;
            continue;
        }

        // Access the bus number directly through the libusb_device structure
        libusb_device *dev = current_device->dev;
        libusb_device_handle *handle = nullptr;

        // Open the device to get a handle
        res = libusb_open(dev, &handle);
        if (res < 0) {
            std::cerr << "Error opening device" << std::endl;
            current_device = current_device->next;
            continue;
        }

        // Get Manufacturer String (string index 1)
        unsigned char manufacturer[256];
        res = libusb_get_string_descriptor_ascii(handle, descriptor.iManufacturer, manufacturer, sizeof(manufacturer));
        if (res < 0) {
            std::cerr << "Error getting manufacturer string" << std::endl;
            manufacturer[0] = '\0';  // Set to empty if failed
        }

        // Get Product String (string index 2)
        unsigned char product[256];
        res = libusb_get_string_descriptor_ascii(handle, descriptor.iProduct, product, sizeof(product));
        if (res < 0) {
            std::cerr << "Error getting product string" << std::endl;
            product[0] = '\0';  // Set to empty if failed
        }

        // Output the full information like dmesg
        std::cout << "uftdi"
                  << descriptor.idVendor << ":"
                  << descriptor.idProduct
                  << " (" << manufacturer << " " << product << ")"
                  << " on usbus" << libusb_get_bus_number(dev)
                  << std::endl;

        // Close the device handle
        libusb_close(handle);

        current_device = current_device->next;
    }

    // Cleanup and deinitialize
    ftdi_deinit(&ftdi);
    return 0;
}