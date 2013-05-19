/*
 * HelloADK - Android Open Accessory sample for Raspberry Pi
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
 
#include "AOA/AOA.h"
#include <bcm2835.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

//Raspberry Pi GPIO
#define LED1 RPI_V2_GPIO_P1_11
#define SW1  RPI_V2_GPIO_P1_12

AOA acc("ammlab.org",
        "HelloADK",
        "DemoKit Arduino Board",
        "1.0",
        "https://play.google.com/store/apps/details?id=org.ammlab.android.helloadk",
        "0000000012345678") ;

//signal handler
void signal_callback_handler(int signum)
{
    printf("\ndetect key interrupt\n",signum);
    bcm2835_close();
    printf("Program exit\n");
    exit(0);
}

int main()
{
    int res;
    unsigned char buf[3];

    if (!bcm2835_init())
        return 1;

    bcm2835_gpio_fsel(LED1, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SW1, BCM2835_GPIO_FSEL_INPT);
    //Input pull-up
    bcm2835_gpio_set_pud(SW1, BCM2835_GPIO_PUD_UP);

    signal(SIGINT, signal_callback_handler);
    printf("press ^C to exit program ...\n");

    acc.connect();
    while(1){
        res = acc.read(buf, 2, 10);
          if(res > 0){
              printf("%d bytes rcvd : %02X %02X\n", res, buf[0], buf[1]);
              if(buf[0] == 0x01){
                  if(buf[1] == 1){
                      bcm2835_gpio_write(LED1, HIGH);
                  }else{
                      bcm2835_gpio_write(LED1, LOW);
                  }
              }
          }else if(res == LIBUSB_ERROR_TIMEOUT ){
          }else{
             break;
          }

          buf[0] = 1;
          //read switch status
          uint8_t value = bcm2835_gpio_lev(SW1);
          if( value == 0 ){
              buf[1] = 1;
          }else{
              buf[1] = 0;
          }
          acc.write(buf, 2, 10);

    }
    bcm2835_close();
    return 0;
}
