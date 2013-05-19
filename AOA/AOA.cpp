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

#include "AOA.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>


#define USB_ACCESSORY_VENDOR_ID             0x18D1
//AOA1.0 Specific
#define USB_ACCESSORY_PRODUCT_ID            0x2D00
#define USB_ACCESSORY_ADB_PRODUCT_ID        0x2D01

//AOA2.0 Specific
#define USB_AUDIO_PRODUCT_ID                0x2D02
#define USB_AUDIO_ADB_PRODUCT_ID            0x2D03
#define USB_ACCESSORY_AUDIO_PRODUCT_ID      0x2D04
#define USB_ACCESSORY_AUDIO_ADB_PRODUCT_ID  0x2D05
#define ACCESSORY_PID 0x2D01
#define ACCESSORY_PID_ALT 0x2D00

#define ACCESSORY_STRING_MANUFACTURER       0
#define ACCESSORY_STRING_MODEL              1
#define ACCESSORY_STRING_DESCRIPTION        2
#define ACCESSORY_STRING_VERSION            3
#define ACCESSORY_STRING_URI                4
#define ACCESSORY_STRING_SERIAL             5

#define ACCESSORY_GET_PROTOCOL              51
#define ACCESSORY_SEND_STRING               52
#define ACCESSORY_START                     53

AOA::AOA(const char *manufacturer,
         const char *model,
         const char *description,
         const char *version,
         const char *uri,
         const char *serial) : manufacturer(manufacturer),
                               model(model),
                               description(description),
                               version(version),
                               uri(uri),
                               serial(serial),
                               handle(NULL),
                               ctx(NULL)
{

}


AOA::~AOA()
{
    if(handle != NULL ){
        libusb_release_interface (handle, 0);
        libusb_close(handle);
    }
    if( ctx != NULL ){
       libusb_exit(ctx);
    }
}

///////////////////////////////////////////////////////////////////////////////
// connect() - connect to AOA device
///////////////////////////////////////////////////////////////////////////////
//argument
// none
//return
//  0 : connection success
// -1 : connection failed.(AOA device not found)
// -2 : connection failed.(Accessory mode switch failed)
// -3 : connection failed.(libusb_claim_interface failed)
//
int AOA::connect(void)
{
    if(libusb_init(&ctx) < 0){
       printf("libusb_init failed\n");
       return 1;
    }
    unsigned char ioBuffer[2];
    int protocol;
    int res;
    int tries = 5;
    uint16_t idVendor, idProduct;

    // Search for AOA support device in the all USB devices
    if ((protocol=searchDevice(ctx, &idVendor, &idProduct)) < 0) {
        printf("AOA device not found.\n");
        return -1;
    }

    //already in accessory mode ?
    if( protocol == 0 ) {
       handle = libusb_open_device_with_vid_pid(ctx, idVendor, idProduct);
       libusb_claim_interface(handle, 0);
       return 0;
    }
    verProtocol = protocol;

    handle = libusb_open_device_with_vid_pid(ctx, idVendor, idProduct);
    libusb_claim_interface(handle, 0);
    usleep(1000);//sometimes hangs on the next transfer :(

    // Send accessory identifications
    sendString(ACCESSORY_STRING_MANUFACTURER, (char*)manufacturer);
    sendString(ACCESSORY_STRING_MODEL, (char*)model);
    sendString(ACCESSORY_STRING_DESCRIPTION, (char*)description);
    sendString(ACCESSORY_STRING_VERSION, (char*)version);
    sendString(ACCESSORY_STRING_URI, (char*)uri);
    sendString(ACCESSORY_STRING_SERIAL, (char*)serial);

    // Switch to accessory mode
    res = libusb_control_transfer(handle,0x40,ACCESSORY_START,0,0,NULL,0,0);
    if(res < 0){
        libusb_close(handle);
        handle = NULL;
        return -2;
    }

    if(handle != NULL){
        libusb_close(handle);
        handle = NULL;
    }

    // Wait a moment
    usleep(10000);

    printf("connect to new PID...\n");
    //attempt to connect to new PID, if that doesn't work try ACCESSORY_PID_ALT
    for(;;){
        tries--;
        if(searchDevice(ctx, &idVendor, &idProduct) != 0 ){
            continue;
        }
        if((handle = libusb_open_device_with_vid_pid(ctx, idVendor, idProduct)) == NULL){
            if(tries < 0){
                return -1;
            }
        }else{
            break;
        }
        sleep(1);
    }

    res = libusb_claim_interface(handle, 0);
    if(res < 0){
        return -3;
    }

    printf("Established AOA connection.\n");
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// read() - read data from accessory device
///////////////////////////////////////////////////////////////////////////////
//argument
// buf     : data buffer
// len     : data length
// timeout : wait time(ms)
//
//return
//  <0 : Error (libusb_bulk_transfer error code)
// >=0 : Succes(received bytes)
//
int AOA::read(unsigned char *buf, int len, unsigned int timeout)
{
   int xferred;
   int res = libusb_bulk_transfer(handle, inEP, buf, len, &xferred, timeout);
   if(res == 0) res = xferred;
   return(res);
}

///////////////////////////////////////////////////////////////////////////////
// write() - write data to accessory device
///////////////////////////////////////////////////////////////////////////////
//argument
// *buf    : data buffer
// len     : data length
// timeout : wait time(ms)
//
//return
//  <0 : Error (libusb_bulk_transfer error code)
// >=0 : Succes(received bytes)
//
int AOA::write(unsigned char *buf, int len, unsigned int timeout)
{
   int xferred;
   int res = libusb_bulk_transfer(handle, outEP, buf, len, &xferred, timeout);
   if(res == 0) res = xferred;
   return(res);
}

///////////////////////////////////////////////////////////////////////////////
// getProtocol() - retrieve AOA protocol version
///////////////////////////////////////////////////////////////////////////////
//argument
// none
//
//return
// -1 : Error (libusb_bulk_transfer error code)
// >0 : Protocol version
//
int AOA::getProtocol()
{
    unsigned short protocol;
    unsigned char buf[2];
    int res;

    res = libusb_control_transfer(handle, 0xc0, ACCESSORY_GET_PROTOCOL, 0, 0, buf, 2, 0);
    if(res < 0){
        return -1;
    }
    protocol = buf[1] << 8 | buf[0];
    return((int)protocol);
}

///////////////////////////////////////////////////////////////////////////////
// sendString() - send accessory identifications
///////////////////////////////////////////////////////////////////////////////
//argument
// index   : string ID
// str     : identification string(zero terminated UTF8 string)
//
//return
// <0 : Error (libusb_bulk_transfer error code)
//  0 : Success
//
int AOA::sendString(int index, const char *str)
{
    int res;
    res = libusb_control_transfer(handle, 0x40, ACCESSORY_SEND_STRING, 0, index, (unsigned char*)str, strlen(str) + 1, 0);
    return(res);
}

///////////////////////////////////////////////////////////////////////////////
// searchDevice() -  Search for AOA support device in the all USB devices
///////////////////////////////////////////////////////////////////////////////
//argument
// ctx      : libusb_context
// idVendor : return buffer for USB Vendor ID
// idProduc : return buffer for USB Product ID
//
//return
// -1 : AOA device not found
// 0> : AOA Version
//
int AOA::searchDevice(libusb_context *ctx, uint16_t *idVendor, uint16_t *idProduct)
{
    int res;
    int i;
    libusb_device **devs;
    libusb_device *dev;
    struct libusb_device_descriptor desc;
    ssize_t devcount;

    *idVendor = *idProduct = 0;
    res = -1;
    devcount = libusb_get_device_list(ctx, &devs);
    if(devcount < 0){
       printf("Get device error.\n");
       return -1;
    }

    //enumerate USB deivces
    for(i=0; i<devcount; i++){
        dev = devs[i];
        libusb_get_device_descriptor(dev, &desc);
#ifdef DEBUG
        printf("VID:%04X, PID:%04X Class:%02X\n", desc.idVendor, desc.idProduct, desc.bDeviceClass);
#endif
        //Ignore non target device
        if( desc.bDeviceClass != 0 ){
            continue;
        }

        //Already AOA mode ?
        if(desc.idVendor == USB_ACCESSORY_VENDOR_ID &&
            (desc.idProduct >= USB_ACCESSORY_PRODUCT_ID &&
             desc.idProduct <= USB_ACCESSORY_AUDIO_ADB_PRODUCT_ID)
        ){
#ifdef DEBUG
            printf("already in accessory mode.\n");
#endif
            res = 0;
            break;
        }

        //Checking the AOA capability.
        if((handle = libusb_open_device_with_vid_pid(ctx, desc.idVendor,  desc.idProduct)) == NULL) {
               printf("Device open error.\n");
        } else {
                libusb_claim_interface(handle, 0);
                res = getProtocol();
                libusb_release_interface (handle, 0);
                libusb_close(handle);
                handle = NULL;
                if( res != -1 ){
#ifdef DEBUG
    printf("AOA protocol version: %d\n", verProtocol);
#endif
                    break; //AOA found.
                }
        }
    }

    // find end point number
    if( findEndPoint(dev) < 0 ){
        printf("Endpoint not found.\n");
        res = -1;
    }
    *idVendor = desc.idVendor;
    *idProduct = desc.idProduct;
#ifdef DEBUG
    printf("VID:%04X, PID:%04X\n", *idVendor, *idProduct);
#endif
    return res;
}

///////////////////////////////////////////////////////////////////////////////
// findEndPoint() -  find end point number
///////////////////////////////////////////////////////////////////////////////
//argument
// dev : libusb_device
//
//return
//  0 : Success
// -1 : Valid end point not found
//
int AOA::findEndPoint(libusb_device *dev)
{
    struct libusb_config_descriptor *config;
    libusb_get_config_descriptor (dev, 0, &config);

    //initialize end point number
    inEP = outEP = 0;

    //Evaluate first interface and endpoint descriptor
#ifdef DEBUG
    printf("bNumInterfaces: %d\n", config->bNumInterfaces);
#endif
    const struct libusb_interface *itf = &config->interface[0];
    struct libusb_interface_descriptor ifd = itf->altsetting[0];
#ifdef DEBUG
    printf("bNumEndpoints: %d\n", ifd.bNumEndpoints);
#endif
    for(int i=0; i<ifd.bNumEndpoints; i++){
        struct libusb_endpoint_descriptor epd;
        epd = ifd.endpoint[i];
        if( epd.bmAttributes == 2 ) { //Bulk Transfer ?
            if( epd.bEndpointAddress & 0x80){ //IN
                if( inEP == 0 )
                    inEP = epd.bEndpointAddress;
            }else{                            //OUT
                if( outEP == 0 )
                    outEP = epd.bEndpointAddress;
            }
        }
#ifdef DEBUG
    printf(" bEndpointAddress: %02X, bmAttributes:%02X\n", epd.bEndpointAddress, epd.bmAttributes);
#endif
    }
    if( outEP == 0 || inEP == 0) {
        return -1;
    }else{
        return 0;
    }
}
