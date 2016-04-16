/*
 *
 *  Developed by Hassan Albalawi (April 2016), Hassan@ksa.us.com
 *
 *  >>>> THIS CODE USED TO STREAM OpenBCI V3_32 DATA TO iPhone App "Alpha Waves" <<<<
 *
 *   *** WHAT DOES IT DO ***
 *
 *  This sketch will stream data from Channel #1 only,
 *
 *
 *
 *
 *   *** DEPENDENCIES ***
 *   In order to make it work, you have to setup the Rfdunio with the attached skecth
 *   (refer to OpenBCI documentary to know how to upload a code to Rfdunio on the board)
 *
 *
 *   *** Credits ***
 *
 *  This sketch uses Biquad filter library, which was originally devloped by:
 *  http://www.earlevel.com/main/2012/11/26/biquad-c-source-code/
 *  Then, it was modified by Chip Audette to be used on OpenBCI 8-bit board.
 *  Then, I modified it to be used on OpenBCI 32-bit Board
 *
 *  The method of calucalting the Alpha power was inspired by the code developed by Chip Aduette here:
 *  https://github.com/chipaudette/EEGHacker/blob/master/Arduino/OBCI_V2_AlphaDetector/
 *
 *  The core of this code is a modified version of the OpenBCI 32-bit original code
 */


#include <DSPI.h>
#include <EEPROM.h>
#include "OpenBCI_32_BLE.h"

//------------------------------------------------------------------------------
//  << OpenBCI BUSINESS >>
boolean is_running = false;    // this flag is set in serialEvent on reciept of ascii prompt
OpenBCI_32_BLE OBCI; //Uses SPI bus and pins to say data is ready.

// these are used to change individual channel settings from PC
char currentChannelToSet;    // keep track of what channel we're loading settings for
boolean getChannelSettings = false; // used to receive channel settings command
int channelSettingsCounter; // used to retrieve channel settings from serial port
int leadOffSettingsCounter;
boolean getLeadOffSettings = false;

// these are all subject to the radio requirements: 31byte max packet length (maxPacketLength - 1 for packet checkSum)
#define OUTPUT_NOTHING (0)  // quiet
#define OUTPUT_BINARY (1)  // normal transfer mode
#define OUTPUT_BINARY_SYNTHETIC (2)  // needs portage
int outputType;

//------------------------------------------------------------------------------
//  << LIS3DH Accelerometer Business >>
//  LIS3DH_SS on pin 5 defined in OpenBCI library
volatile boolean auxAvailable = false;
volatile boolean addAccel = false;
boolean useAccelOnly = false;
//------------------------------------------------------------------------------
//  << PUT FILTER STUFF HERE >>
#define MAX_N_CHANNELS (8)   //how many channels are available in hardware
#define N_EEG_CHANNELS (1)
//Design filters  (This BIQUAD class requires ~6K of program space!  Ouch.)
//For frequency response of these filters: http://www.earlevel.com/main/2010/12/20/biquad-calculator/
#include "Biquad.h"   //modified from this source code:  http://www.earlevel.com/main/2012/11/26/biquad-c-source-code/

// Stop DC filter
#define SAMPLE_RATE_HZ (250.0)  //default setting for OpenBCI
#define FILTER_Q (0.5)        //critically damped is 0.707 (Butterworth)
#define PEAK_GAIN_DB (0.0) //we don't want any gain in the passband
#define HP_CUTOFF_HZ (0.5)  //set the desired cutoff for the highpass filter
Biquad stopDC_filter(bq_type_highpass, HP_CUTOFF_HZ / SAMPLE_RATE_HZ, FILTER_Q, PEAK_GAIN_DB); //one for each channel because the object maintains the filter states

// Notch Filter
#define NOTCH_FREQ_HZ (50.0)      // Make sure you select the right power line frequency; set it to 60Hz if you're in the U.S.
#define NOTCH_Q (4.0)              //pretty sharp notch
Biquad notch_filter1(bq_type_notch, NOTCH_FREQ_HZ / SAMPLE_RATE_HZ, NOTCH_Q, PEAK_GAIN_DB); //one for each channel because the object maintains the filter states
Biquad notch_filter2(bq_type_notch, NOTCH_FREQ_HZ / SAMPLE_RATE_HZ, NOTCH_Q, PEAK_GAIN_DB); //one for each channel because the object maintains the filter states

// Design signal detection filter

//////////////////////////// Start Power Filters //////////////////////////////////
#define BP_Q (2.0f) //somewhat steep slope

// ALPHA High Power (10 - 11.75Hz)
#define AHP_FREQ_HZ (10.0f)  //focus on High Alpha waves
Biquad AHP_bandpass_filter1(bq_type_bandpass, AHP_FREQ_HZ / SAMPLE_RATE_HZ, BP_Q, PEAK_GAIN_DB); //one for each channel because the object maintains the filter states
Biquad AHP_bandpass_filter2(bq_type_bandpass, AHP_FREQ_HZ / SAMPLE_RATE_HZ, BP_Q, PEAK_GAIN_DB); //one for each channel because the object maintains the filter states

// BETA High Power (18 - 29.75Hz)
#define BHP_FREQ_HZ (25.0f)  //focus on High Beta waves
Biquad BHP_bandpass_filter1(bq_type_bandpass, BHP_FREQ_HZ / SAMPLE_RATE_HZ, BP_Q, PEAK_GAIN_DB); //one for each channel because the object maintains the filter states
Biquad BHP_bandpass_filter2(bq_type_bandpass, BHP_FREQ_HZ / SAMPLE_RATE_HZ, BP_Q, PEAK_GAIN_DB); //one for each channel because the object maintains the filter states

Biquad *AHP_bp1, *AHP_bp2, *BHP_bp1, *BHP_bp2;
////////////////////////////// End Power Filters ////////////////////

#define MICROVOLTS_PER_COUNT (0.02235174f)  //Nov 10,2013...assumes gain of 24, includes mystery factor of 2... = 4.5/24/(2^24) *  2


//define some output pins
int BUZZER_OUTPUT_PIN = 13;
int Red_OUTPUT_PIN  = 12;

boolean AlphaDetector = true;  //enable or disable as you'd like..
boolean NoBase = false; // If you would like to establish a base instead of using the standard based (was set experimently)
boolean cancelNoise = true; // Do you want to cancel noise by Beta-wave method?
boolean DetectNoise = false; // Send error code if you recieved a lot of noises
boolean UseStandardBase = true; // Used if you don't establish a base
boolean printAlpha = false; // used to print Alpha value instead of counter value when true
//------------------------------------------------------------------------------

int LED = 11;  // blue LED alias
int PGCpin = 12;  // PGC pin goes high when PIC is in bootloader mode
//------------------------------------------------------------------------------

void setup(void) {

  Serial0.begin(9600);  // using hardware uart number 0
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);   // blue LED
  pinMode(PGCpin, OUTPUT);
  digitalWrite(PGCpin, LOW); // used to tell RFduino if we are in bootloader mode

  startFromScratch();

  //setup other pins

  // RGB Pins
  pinMode(Red_OUTPUT_PIN, OUTPUT);
  pinMode(BUZZER_OUTPUT_PIN, OUTPUT);

  // on then off to make sure they are working properly
  digitalWrite(Red_OUTPUT_PIN, HIGH);
  delay(500);
  digitalWrite(Red_OUTPUT_PIN, LOW);
  digitalWrite(BUZZER_OUTPUT_PIN, HIGH);
  delay(1000);
  digitalWrite(BUZZER_OUTPUT_PIN, LOW);


  // Turn off all channels except channel #1 in AlphaDetector Mode
  if (AlphaDetector) {
    changeChannelState_maintainRunningState(2, DEACTIVATE);
    changeChannelState_maintainRunningState(3, DEACTIVATE);
    changeChannelState_maintainRunningState(4, DEACTIVATE);
    changeChannelState_maintainRunningState(5, DEACTIVATE);
    changeChannelState_maintainRunningState(6, DEACTIVATE);
    changeChannelState_maintainRunningState(7, DEACTIVATE);
    changeChannelState_maintainRunningState(8, DEACTIVATE);
  }
}

int prev_LED_val = 0, LED_val = 0;

void loop() {


  if (is_running) {

    while (!(OBCI.isDataAvailable())) {
    }   // wait for DRDY pin...

    OBCI.updateChannelData(); // get the fresh ADS results

    if (AlphaDetector)  Run_AlphaDetector();
  }
  eventSerial();
}


// some variables to help find 'burger protocol' commands
int plusCounter = 0;
char testChar;
unsigned long commandTimer;

void eventSerial() {
  if (Serial0.available() > 0) {
    char inChar = (char)Serial0.read();

    getCommand(inChar);
  }
}


void getCommand(char token) {
  switch (token) {
    //TURN CHANNELS ON/OFF COMMANDS
    case '1':
      changeChannelState_maintainRunningState(1, DEACTIVATE);
      break;
    case '2':
      changeChannelState_maintainRunningState(2, DEACTIVATE);
      break;
    case '3':
      changeChannelState_maintainRunningState(3, DEACTIVATE);
      break;
    case '4':
      changeChannelState_maintainRunningState(4, DEACTIVATE);
      break;
    case '5':
      changeChannelState_maintainRunningState(5, DEACTIVATE);
      break;
    case '6':
      changeChannelState_maintainRunningState(6, DEACTIVATE);
      break;
    case '7':
      changeChannelState_maintainRunningState(7, DEACTIVATE);
      break;
    case '8':
      changeChannelState_maintainRunningState(8, DEACTIVATE);
      break;
    case '!':
      changeChannelState_maintainRunningState(1, ACTIVATE);
      break;
    case '@':
      changeChannelState_maintainRunningState(2, ACTIVATE);
      break;
    case '#':
      changeChannelState_maintainRunningState(3, ACTIVATE);
      break;
    case '$':
      changeChannelState_maintainRunningState(4, ACTIVATE);
      break;
    case '%':
      changeChannelState_maintainRunningState(5, ACTIVATE);
      break;
    case '^':
      changeChannelState_maintainRunningState(6, ACTIVATE);
      break;
    case '&':
      changeChannelState_maintainRunningState(7, ACTIVATE);
      break;
    case '*':
      changeChannelState_maintainRunningState(8, ACTIVATE);
      break;
    case '0':
    case '-':
    case '=':
    case 'p':
    case '[':
    case ']':
    case'A':
      DetectNoise = false;
      //    Serial0.println("DetectNoise is False");
      break;
    case'S':
      DetectNoise = true;
      //    Serial0.println("DetectNoise is True");
      break;
    case'F':
      UseStandardBase = false;
      //    Serial0.println("UseStandardBase is False");
      break;
    case'G':
      UseStandardBase = true;
      //    Serial0.println("UseStandardBase is True");
      break;
    case'H':
      cancelNoise = false;
      //    Serial0.println("cancelNoise is False");
      break;
    case'J':
      cancelNoise = true;
      //    Serial0.println("cancelNoise is True");
      break;
    case'K':
    case'L':
    case 'a':
    case 'h':
    case 'j':
    // CHANNEL SETTING COMMANDS
    case 'x':
    case 'X':
    case 'd':
      // used to test the connection
      digitalWrite(LED, HIGH);
      break;
    case 'D':
    case 'c':
      // used to test the connection
      digitalWrite(LED, LOW);
      break;
    case 'C':
    case 'z':
    case 'Z':
    // STREAM DATA AND FILTER COMMANDS
    case 'v':
      //      startFromScratch();
      break;
    case 'n':
    case 'N':
    case 'b':  // stream data
      startRunning(OUTPUT_BINARY);
      break;
    case 's':  // stop streaming data
      OBCI.disable_accel();
      stopRunning();
      break;
    case 'f':
      printAlpha = true;
      break;
    case 'g':
      printAlpha = false;
      break;
    case '?':
    default:
      break;
  }
}// end of getCommand

boolean stopRunning(void) {
  if (is_running == true) {
    OBCI.stopStreaming();                    // stop the data acquisition  //
    is_running = false;
  }
  return is_running;
}

boolean startRunning(int OUT_TYPE) {
  if (is_running == false) {
    outputType = OUT_TYPE;
    OBCI.startStreaming();
    is_running = true;
  }
  return is_running;
}

int changeChannelState_maintainRunningState(int chan, int start)
{
  boolean is_running_when_called = is_running;
  int cur_outputType = outputType;

  //must stop running to change channel settings
  stopRunning();
  if (start == true) {
    //    Serial0.print("Activating channel ");
    //    Serial0.println(chan);
    OBCI.activateChannel(chan);
  }
  else {
    //    Serial0.print("Deactivating channel ");
    //    Serial0.println(chan);
    OBCI.deactivateChannel(chan);
  }
  //restart, if it was running before
  if (is_running_when_called == true) {
    startRunning(cur_outputType);
  }
}
//
void startFromScratch() {
  delay(1000);
  //  Serial0.print("OpenBCI V3 32bit Board\nSetting ADS1299 Channel Values\n");
  OBCI.useAccel = false;  // option to add accelerometer dat to stream
  OBCI.useAux = false;    // option to add user data to stream not implimented yet
  OBCI.initialize();
  OBCI.configureLeadOffDetection(LOFF_MAG_6NA, LOFF_FREQ_31p2HZ);
  //  Serial0.print("ADS1299 Device ID: 0x");
  //  Serial0.println(OBCI.ADS_getDeviceID(),HEX);
  //  Serial0.print("LIS3DH Device ID: 0x");
  //  Serial0.println(OBCI.LIS3DH_getDeviceID(),HEX);
  //  sendEOT();
}

// DO FILTER STUFF HERE IF YOU LIKE
float counter = 100;
int ind_count = 1;
float AHP_tmp[250];
float BHP_tmp[250];
float Diff_H = 0;
float AHP = 0;
float BHP = 0;
float A_HST = 0.0f;
float A_HWT = 0.0f;
float base_HST[35];
float array_BHT[35];
float base_AHST = 0;
float base_BHT = 0;
int base_ind = 0;
int noise_ind = 0;
float B_Noise = 50;

void Run_AlphaDetector(void) {
  float val, DP_val, TP_val, ALP_val, AHP_val, BLP_val, BHP_val, rms_val;
  // float DP_val2,TP_val2,ALP_val2,AHP_val2,BLP_val2,BHP_val2,rms_val2;
  //loop over each channel
  // Because we are reading the data directly the channels index is from [0 to 7]
  val = (float) OBCI.channelDataInt[0]; //get the stored value for this sample from channel#1 (its index = 0)

  //apply DC-blocking highpass filter
  val = stopDC_filter.process(val);    //apply DC-blocking filter

  //apply 50Hz notch filter...twice to make it more effective
  val = notch_filter1.process(val);     //apply 50Hz notch filter
  val = notch_filter2.process(val);     //apply 50Hz notch again
  //put this data back into ADSManager in case we want to output i tusing the ADSManager buit-in output functions
  OBCI.channelDataInt[0] = (long) val;

  // store the valued to be used by different bands
  float val_common = val;
  //////////// Compute the power from different bands //////////////

  AHP_val = val_common;
  AHP_bp1 = &AHP_bandpass_filter1;
  AHP_bp2 = &AHP_bandpass_filter2;
  AHP_val = AHP_bp1->process(AHP_val);    //apply bandpass filter
  AHP_val = AHP_bp2->process(AHP_val);    //do it again to make it even tighter


  // Get Beta value for noise cancelation
  BHP_val = val_common;
  BHP_bp1 = &BHP_bandpass_filter1;
  BHP_bp2 = &BHP_bandpass_filter2;
  BHP_val = BHP_bp1->process(BHP_val);    //apply bandpass filter
  BHP_val = BHP_bp2->process(BHP_val);    //do it again to make it even tighter


  //---------------- Update Counter & Act on data ------------------------//

  // Scale the data and save it
  AHP_tmp[ind_count] = AHP_val * AHP_val;
  BHP_tmp[ind_count] = BHP_val * BHP_val;

  if (ind_count == 250) { // if we reached a full second

    // Calculate RMS value from 250 values (One Second)
    for (int i = 1; i < 251; i++) {
      AHP = AHP + AHP_tmp[i];
      BHP = BHP + BHP_tmp[i];
    }


    AHP =  sqrt(abs(AHP / 250.0f));
    BHP =  sqrt(abs(BHP / 250.0f));


    Diff_H = (AHP * MICROVOLTS_PER_COUNT); // - (BHP*MICROVOLTS_PER_COUNT);
    //
    //    //------------ Noise Detection & Faulty Wire ------------//
    //    //If the value is too high, skip this sample
    if (cancelNoise) {
      if (BHP * MICROVOLTS_PER_COUNT > B_Noise) {
        ind_count = 1;
        noise_ind++;
        // Setup a lot of noise detection, it might be a sign of faulty wire, placement .. etc
        if (noise_ind > 10) { // if noise last for 10 seconds
          if (DetectNoise) {
            float error_code = 911; // Use this to tell that we're receiving a lot of noise!
            Serial0.write((float) error_code);
          }
        }
        return;
      }
    }
    else {
      noise_ind = 0;
    }
    //---------- END: Noise Detection & Faulty Wire ----------//

    if (UseStandardBase) {
      A_HWT = 5.0f;
      A_HST = 6.0f;
    }
    else {
      // If no base
      if (NoBase) {
        base_HST[base_ind] = Diff_H;
        array_BHT[base_ind] = BHP * MICROVOLTS_PER_COUNT;
        if (base_ind == 34) {
          for (int i = 5; i < 35; i++) {
            base_AHST = base_AHST + base_HST[i];
            base_BHT = base_BHT + array_BHT[i];
          }
          A_HWT = (base_AHST / 30);
          A_HST = A_HWT + 3;

          B_Noise =  (base_BHT / 30) + 5;

          NoBase = false;

          //          digitalWrite(MidAlert_OUTPUT_PIN, LOW);
        }
        else {
          base_ind++;
        }
      }
    }
    if (UseStandardBase || !NoBase) {
      //--------- Update Counter & Send Alert -------------//
      if (Diff_H > A_HST) {
        counter -= 20;
        if (counter < 0) {
          counter = 0;
        }
      }
      if (Diff_H < A_HWT) {
        counter += 20;
        if (counter > 100) {
          counter = 100;
        }
      }
      // update LED value with counter value
      if (counter >= 70) {
        digitalWrite(Red_OUTPUT_PIN, LOW);
      }
      else if (counter < 70 & counter > 30) {
        digitalWrite(Red_OUTPUT_PIN, HIGH);
        digitalWrite(BUZZER_OUTPUT_PIN, LOW);
      }
      else if (counter <= 30) {
        digitalWrite(BUZZER_OUTPUT_PIN, HIGH);
        //sendAlert();
      }
    }
    //--------- End: Update Counter ----------//
    ind_count = 1;
    if (printAlpha == true) {
      Serial0.write((float) Diff_H);
    } else {
      Serial0.write((float) counter);
    }
  } // End of 250samples loop

  OBCI.channelDataInt[4] = (long) AHP;
  OBCI.channelDataInt[6] = (long) BHP;
  OBCI.channelDataInt[7] = (long) counter;
  OBCI.update24bitData();

  ind_count++;
  //---------------- END: Update Counter & Act on data ------------------------//
}





