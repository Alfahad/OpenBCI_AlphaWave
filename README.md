# OpenBCI AlphaWave
Measure your Alpha Wave using OpenBCI 32-bit board with iOS App. This is a simple demo to demonstrate the ability of OpenBCI board to:

- **Process data onboard.**
- **Communicate with mobile apps via Bluetooth directly (iOS & Android).**

In order to get this demo working, we need to customized the software at:

1. **OpenBCI Board Microcontroller ([How to?](http://docs.openbci.com/tutorials/02-Upload_Code_to_OpenBCI_Board))**
2. **OpenBCI Radio ([How to?](http://docs.openbci.com/tutorials/03-Upload_Code_to_OpenBCI_Dongle))**
3. **Install mobile app from the source codes**

In the learning section of OpenBCI, you can find all the tutorial you need to know how to upload the customized codes to the board & radios. I'm not going over those tutorials again; instead, I'll focus on the communicating with the board & the algorithm used to process the data.

In order to communicate with the board via bluetooth, we need to setup the board radio, RFduino, to receive data from the board & stream it out to the Mobile App. Also, we need to setup the board to receive data, commands, from the App and carry it to the board. Lastly, just install the App in your mobile device using the source code.

## **Process Data Onboard**

The 32bit OpenBCI Board implements the [PIC32MX250F128B](http://www.microchip.com/wwwproducts/Devices.aspx?dDocName=en557425) microcontroller, which comes with lots of local memory and fast processing speeds. The OpenBCI 32-bit board comes already pre-flashed with the chipKIT™ bootloader, and the latest OpenBCI firmware. This software is intended to stream raw data from the board to a PC via two radios modules; Rfduino on board & Rfduino on the USB dongle. However, we can customize the microcontroller's code to process the raw data, act on data, and do many things :)

When I was working on this project, I had more than one idea to demonstrate this work, but I decided to work on a frequency-based BCI application. In particular, to measure the Alpha amplitude since it almost everyone (~85%) of us can produce Alpha waves when we close our eyes; it's even stronger when EEG is read from the occipital region of the brain. Therefore, this is exactly what I've done here. To make a sense of the Alpha measures, I related it to user's alertness; by saying higher Alpha indicates lower alert level because you're eyes are closed. This is just an assumption for the purpose of this demo, because user's alertness cannot be really defined by a single measures; it's more complicated than that.

I like the approach done by the EEGHacker to [detect Alpha Waves](http://eeghacker.blogspot.qa/2013/11/openbci-alpha-wave-detector.html) using OpenBCI 8-bit version. It's a straightforward approach to pre-process raw data, measure Alpha amplitude, and act on the data. I've followed a similar approach in pre-processing data, but I implemented a counter-based method to act on data. Also, I implemented a method to create a baseline in order to act on data properly for different users; since we don't produce the same amplitude of Alpha. The following flowchart diagram illustrate the algorithm flow:  
[![General Flowchart - New Page](http://openbci.com/community/wp-content/uploads/2016/04/General-Flowchart-New-Page-e1460833325624-844x1024.png)](http://openbci.com/community/wp-content/uploads/2016/04/General-Flowchart-New-Page-e1460831192684.png)

The counter-based approach is implemented, which look into the Alpha amplitude if it's higher/lower than a specific threshold to increase/decrease the counter. Those thresholds are established in the baseline step, but the default setting is using a pre-determined thresholds. They worked for me & some of my friends too, but it might be different for other people; therefore, the algorithm can determine them if the baseline is enabled by setting "boolean UseStandardBase = false" in the code.

The following chart illustrate the counter-based approach:

For more details, please go through the [source code](https://github.com/Alfahad/OpenBCI_AlphaWave/tree/master/Board%20Codes). If you have any question, please post them here in the comment section.

## **AlphaWave Setup**

Here I would like to go over the steps in general; assuming that the original firmware is already on the board, the board radio, and USB dongle:

1. **Upload the [board code](https://github.com/Alfahad/OpenBCI_AlphaWave/tree/master/Board%20Codes) to your OpenBCI 32-bit by following [this tutorial](http://docs.openbci.com/tutorials/02-Upload_Code_to_OpenBCI_Board).**
2. **Upload the [radio code](https://github.com/Alfahad/OpenBCI_AlphaWave/tree/master/Radio%20Codes) to the device radio by following [this tutorial](http://docs.openbci.com/tutorials/03-Upload_Code_to_OpenBCI_Dongle). _Note that if you need to update the board code (step#1), you need to upload the original radio code in order for it to communicate with the dongle_.**
3. **Install mobile app using the source code. _You can use Android Studio for Android-based devices & Xcode for iOS-based devices._**

For this demo, I'm using only channel#1 & the location of the electrode is O1 according to the [10-20 system](http://en.wikipedia.org/wiki/10-20_system_%28EEG%29), which is the same location used by the [Getting Started tutorial](http://docs.openbci.com/tutorials/01-GettingStarted). But instead of a gel-based electrode, I'm using this [reusable Dry EEG Electrode](https://fri-fl-shop.com/product/tde-200/). To hold electrode in a place, I've attached it to a base-ball hat. Also, I'm using another dry electrode placed on T3 according to the [10-20 system](http://en.wikipedia.org/wiki/10-20_system_%28EEG%29) and connected the SRB2 as a reference. Even though, I wasn't connecting my reference electrode to my earlobe, I was able to read a clear Alpha because the potential difference is noticeable enough. One last thing, I didn't use BIAS channel because it didn't make a big difference in my case of using a single channel.

## **AlphaWave Apps**

This demo includes an app for [Android-based devices](https://github.com/Alfahad/OpenBCI-AlphaWave-Android) and [iOS-based devices](https://github.com/Alfahad/OpenBCI-AlphaWave-iOS). The functionality of reading the AlphaWave exists in both apps. However, the android version includes more functions; such as:

- **Ability to send custom commands to the app.**
- **Plotting the alert level on real-time**
- **Summarize the results of each trial**


## **Video**
Here is a video shows this demo in operation:

I hope you've enjoyed this demo.

 
# TO DO
- Upload iOS App to the App Store
- Work on the Android App
- Add enable baseline button to establish the correct thresholds
