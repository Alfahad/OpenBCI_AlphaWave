# OpenBCI AlphaWave
Measure your Alpha Wave using OpenBCI 32-bit board with iOS App

# Video of the Demo
https://www.youtube.com/watch?v=SLth9WrIz3I


# What does it do
This demonstration serves the basics for:
- How to process data on the OpenBCI 32-bit board
- How to communicate with the board via Bluetooth
- How to connect Rfdunio to an iOS App

You can use this demo for many other applications with your OpenBCI board. If you don't have one, get yours now from: http://www.OpenBCI.com

# Dependencies
In order to make this demo works, you have to:

- Setup your OpenBCI 32-bit board using the attached sketch [/OpenBCI_32bit_BLE/](/OpenBCI_32bit_BLE/) (Make sure you also use the modified library, [/Arduino165_Libraries/](/Arduino165_Libraries/) ) Refer to [How Upload Code to OpenBCI Board](http://docs.openbci.com/tutorials/02-Upload_Code_to_OpenBCI_Board)

- Then, you need to setup the RFduino on the baord by uploading this sketch [/OpenBCI_32bit_Device_BLE](/OpenBCI_32bit_Device_BLE/) Refer to [How to Program the Device Radio](http://docs.openbci.com/tutorials/03-Upload_Code_to_OpenBCI_Dongle#uploading-device-firmware-to-openbci-boards-program-device-radio-with-openbci-dongle)

- Install iOS App

# TO DO
- Upload iOS App to the App Store
- Work on the Android App
