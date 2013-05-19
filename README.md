RPi_ADK - Android Open Accessory implementation for Raspberry Pi
==================================================================

This is Android Open Accessory(aka ADK) library package.

Use with libusb-1.0.

Example
-----------
+ HelloADK

  requirement(install to /usr/local)    
  + libusb-1.0
  + bcm2835 library


Limitation
-----------
+ Need root permission
+ Support AOA1.0 only

Disclaimer
----------
This software is no warranty.

We are not responsible for any results caused by the use of this software.

Please use the responsibility of the your self.

License
-------
Copyright &copy; 2013 Yuuichi Akagawa

Licensed under the [Apache License, Version 2.0][Apache]


-----------------------------------------------------------------

RPi_ADK - Android Open Accessory implementation for Raspberry Pi
==================================================================

Raspberry PiでADK接続を実現するライブラリです。

libusb-1.0を利用しているため、Linuxであれば動作するはずです。

サンプルプログラム
-------------------
+ HelloADK

  以下のライブラリが/usr/localにインストールされていることを前提としています   
  + libusb-1.0
  + bcm2835 library

  makeして出来上がったHelloADKを実行します。   
  sudo ./HelloADK

制限
----
+ 実行にはroot権限が必要です
+ AOA1.0のみサポートしています

免責事項
--------
本ソフトウェアは無保証です。
ADK勉強会(東京)は本ソフトウェアの利用によって生じた結果に対して一切の責任を負いません。利用者の自己責任のもとご利用ください。

著作権について
--------------
Copyright &copy; 2013 Yuuichi Akagawa

Licensed under the [Apache License, Version 2.0][Apache]

[Apache]: http://www.apache.org/licenses/LICENSE-2.0
