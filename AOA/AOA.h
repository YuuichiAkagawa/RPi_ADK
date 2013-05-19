/*
 * Android Open Accessory implementation for libusb-1.0
 * Copyright (C) 2013 Yuuichi Akagawa
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __AOA_h__
#define __AOA_h__

#include <libusb.h>

class AOA {
private:
    const char *manufacturer;
    const char *model;
    const char *description;
    const char *version;
    const char *uri;
    const char *serial;

    libusb_context *ctx;
    struct libusb_device_handle* handle;
    uint8_t inEP;
    uint8_t outEP;
    int verProtocol;
    
    int searchDevice(libusb_context *ctx, uint16_t *idVendor, uint16_t *idProduct);
    int findEndPoint(libusb_device *dev);
    int isAccessoryDevice (libusb_context *ctx);
    int getProtocol();
    int sendString(int index, const char *str);

public:
    AOA(const char *manufacturer,
        const char *model,
        const char *description,
        const char *version,
        const char *uri,
        const char *serial);
    ~AOA();
    int connect(void);
    int read(unsigned char *buf, int len, unsigned int timeout);
    int write(unsigned char *buf, int len, unsigned int timeout);
};
#endif /* __AOA_h__ */
