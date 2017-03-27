// OSM 2.1 (one-way) chip communication via LiFi Proof of Concept
// Reciever example
///////////////////////////
/* The MIT License

Copyright 2017 Edsel Valle

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
 */

/* NOTES:
 *  
 *  A lot of people have been saying LiFi was possible but never found any implementations when going through all OSM related
 *  posts I could find. Which I found surprise since finding the below thread:
 *    https://osf.guru/viewtopic.php?f=30&t=20
 *  I learned that OSM LiFi wouldn't need to be as hacky as it seems at the surface. 
 *  
 *  
 *  "GREEN-ADC-IN" (pin A1) is new to OSM 2.1, thus a 2.1 is "required", using other versions is possible I've read.
 *    Most sensitive at lower wavelength (blue end)
 *    Max senstivity when  wavelength <= 490 nm
 *    
 *    DataStream format:
 *      0:            HEADER, 
 *      1-SIZE_DATA:  length of data to read
 *      SIZE_DATA:    CRC Checksum byte
 *      
 *      Go HIGH.
 *      Wait a quarter period.
 *      (READ now for bit value)
 *      bitToWrite==1 -> stay HIGH for another period/4
 *      go LOW
 *      Wait out what's left out of 1 period. (time between rising edge to rising edge is 1 period) 
 *      
 *      
 *      SIZE_DATA and FREQUENCY/PERIOD need not be hardcoded. They are for this example.
 *      
 *      Works at 8 Hz (1 bit of data per cycle), becomes lossy at 16 Hz.
 *        Probably due to the capacatence of the LED. Fall time >> rise time.
 *        Could still probably push to 32 Hz if someone finds a better threshholds for triggering the rising and falling edge. 
 *          Use a scope to find the half power points.
 *      SIZE_AVERAGES should probably be adjusted when adjusting thresholds or the delay between reads.
 *      
 *      Creating a more compact mode format would be great too.
 *      
 *      Hold led to within some inchces to the transmitter (Which can be anything that can programmatically flash blueish light. Brighter is better)
 *      
 *      This could be integrated into Vectr.
 *        If the mode read is written to SRAM at runtime, it'll be erased upon sleep.  
 *        So, would have to be written to EEPROM at runtime, but EEPROM isn't really made to large data, EEPROM is only 1KB, and is access 1 byte at a time.
 *        Optiboot supports writting to flash at runtime, but that's a bit more involved.
 *        Adding an "'import mode' mode" is trivial, but timeings, delays and the like, not so much.
 */
///////////////////

#define PIN_LED_BLUE            5
#define PIN_LED_GREEN_ADCIN     A1

#define SIZE_DATA                 8     //quick
//#define SIZE_DATA               191   //long, size of Vectr's modes
#define FREQUENCY               8     //Hz
#define DELAY_READS             1     //ms
#define NUM_READS               10    //per average
const unsigned int PERIOD = 1000/FREQUENCY;   //ms

const byte HEADER = 200;   

const unsigned int SIZE_AVERAGES = PERIOD/10;
unsigned int averagesSize = 0;
unsigned int averages[SIZE_AVERAGES] = {0};
byte dataSignal[SIZE_DATA + 2] = {0};
unsigned int dataSignalSize = 0;
boolean dataSignalValue = LOW;
long nextReadTime = 2e9; //in millis  //arbitrary high value

//Higher value corresponds to a higher amount of lowerend visible light sensed
unsigned int readLight() {
  unsigned int sum = 0;
  for(byte i=0;i<NUM_READS;i++) {
    sum += analogRead(PIN_LED_GREEN_ADCIN);
    delay(DELAY_READS);
  }
  return sum/NUM_READS;
}

//byte array ouputstream through the LED
void broadcastData(byte data[], unsigned int dataSize) {
  for(int i=0;i<dataSize;i++) {
    for(int j=7;j>=0;j--) {
      digitalWrite(PIN_LED_BLUE, HIGH);
      delay(PERIOD/4);
      digitalWrite(PIN_LED_BLUE, bitRead(data[i], j));
      delay(PERIOD/2);
      digitalWrite(PIN_LED_BLUE, LOW);
      delay(PERIOD/4); 
    }
  }
}

//checksum
//CRC-8 - based on the CRC8 formulas by Dallas/Maxim
//code released under the therms of the GNU GPL 3.0 license
byte CRC8(const byte *data, byte len) {
  byte crc = 0x00;
  while (len--) {
    byte extract = *data++;
    for (byte tempI = 8; tempI; tempI--) {
      byte sum = (crc ^ extract) & 0x01;
      crc >>= 1;
      if (sum) {
        crc ^= 0x8C;
      }
      extract >>= 1;
    }
  }
  return crc;
}

void setup() {
  pinMode(PIN_LED_BLUE , OUTPUT);
  digitalWrite(PIN_LED_BLUE , LOW);
  Serial.begin(9600);
}

void loop() {
  //add average to averages array and shift
  unsigned int average = readLight();
  memmove(averages, averages+1, (SIZE_AVERAGES-1)*sizeof(*averages));
  averages[SIZE_AVERAGES-1] = average;
  if(averagesSize++ < SIZE_AVERAGES) { //accumlate averages
    return;
  }

  //Average of the averages array
  unsigned int averagesAverage = 0;
  for(int i=0;i<SIZE_AVERAGES;i++) {
    averagesAverage += averages[i];
  }
  averagesAverage /= SIZE_AVERAGES; 

  //update dataline's current digital value (dataSignalValue)
  const unsigned int threshhold = 10;
  if(!dataSignalValue && (average > (averagesAverage + threshhold))) {
    dataSignalValue = HIGH;
    //readData
    //keep only the earliest time
    nextReadTime = min(nextReadTime, millis()+PERIOD/2);
  }
  else if(dataSignalValue && (average < (averagesAverage - threshhold))) {
    dataSignalValue = LOW; 
  } 
  //else dataSignalValue retains its previous value

  //if it's time to read the next bit of the datastream
  if(nextReadTime < millis()) {
    Serial.println(dataSignalValue);
    nextReadTime = 2e9;// reset next read time, arbitrary large value

    //update the binarysquence (dataSignal)
    if(dataSignalSize < (SIZE_DATA+2)*8) {
      dataSignal[dataSignalSize/8] |= (dataSignalValue<<((7)-dataSignalSize%8));
      dataSignalSize++;
    }
    else { //left shift & update the binarysquence (dataSignal)
      for(int i=0;i<SIZE_DATA+1;i++) {
        dataSignal[i] = (dataSignal[i]<<1) | bitRead(dataSignal[i+1],7);  
      }
      dataSignal[SIZE_DATA+1] = (dataSignal[SIZE_DATA+1]<<1) | dataSignalValue;  
    }
    //dataSignal[0] is before dataSignal[i] chronologically in time

    //wait for the intiating header to the data to be read
    boolean correctHeader = (dataSignal[0] == HEADER);
    //this headerCheck is actually "late", would have been better to monitor the newest byte read, but for the first SIZE_DATA+2 bytes read,
    //it's not in a fixed spot so this works for now.
    //As a consequence, if you miss the header, you'll have to wait a complete cycle to finish reading.
    if(correctHeader && dataSignalSize >= SIZE_DATA+2) {
      //enough data would have already been read at this point
      Serial.println("Header found. Begin mode data transfer");

      byte readData[SIZE_DATA];
      memmove(readData, dataSignal+1, SIZE_DATA*sizeof(*dataSignal));
      byte crc = CRC8(readData, SIZE_DATA);
      if(dataSignal[SIZE_DATA+1] == crc) {
        Serial.println("Transfer complete. Rebroadcasting the data.");
        while(true) {
          broadcastData(dataSignal, SIZE_DATA+2); 
        }
      }
      else {
        Serial.println("Incorrect checksum. Continuing to read.");
      }
    }
  }
}
