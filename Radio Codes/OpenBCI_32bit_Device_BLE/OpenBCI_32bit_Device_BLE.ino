/*
The sketch was made for the RFduino unit at the OpenBCI 32-bit Board,
to communicate with the OpenBCI-App (see the blog for more details)

Made by Hassan Albalawi, April 2016
Free to use and share. This code presented as-is. No promises!
*/

#include <RFduinoBLE.h>
//int resetPin = ;  // GPIO4

void setup() {
  // this is the data we want to appear in the advertisement
  // (if the deviceName and advertisementData are too long to fix into the 31 byte
  // ble advertisement packet, then the advertisementData is truncated first down to
  // a single byte, then it will truncate the deviceName)
  RFduinoBLE.advertisementData = "OBCI";

  // start the BLE stack
  RFduinoBLE.begin();

  Serial.begin(9600, 3, 2);
  //  Serial.begin(9600);

}

void loop() {

  // send data only when you receive data:
  if (Serial.available() > 0) {
    // read the incoming byte:
    float counter_val = Serial.read();
    RFduinoBLE.sendFloat(counter_val);
  }
}

// Used to send the recieved data from iPhone to the board
void RFduinoBLE_onReceive(char *data, int len)
{
  uint8_t myByte = data[0]; 
  Serial.write(myByte);
}
