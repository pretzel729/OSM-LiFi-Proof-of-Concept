// OSM 2.1 (one-way) chip communication via LiFi Proof of Concept
// Transmiter example
/*
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
 *    DataStream:
 *      0:            HEADER, 
 *      1-SIZE_DATA:  length of data to read
 *      SIZE_DATA:    CRC Checksum byte      
 *      
 *      Go HIGH.
 *      Wait a quarter period.
 *      bitToWrite==1 -> stay HIGH for another period/4
 *      go LOW
 *      Wait out what's left out of 1 period. (time between rising edge to rising edge is 1 period) 
 */
///////////////////

#define PIN_LED_BLUE        5
#define SIZE_DATA           8 //quick
//#define SIZE_DATA           191 //long, size of Vector's modes
#define FREQUENCY           8
const unsigned int PERIOD = 1000/FREQUENCY;     //ms
const byte HEADER = 200;

//quick example
byte dataContent[SIZE_DATA] = {0, 20, 40, 60, 80, 100, 120, 140};
//long example
//byte dataContent[SIZE_DATA] = {0, 2, 0, 10, 0, 5, 0, 0, 0, 0, 0, 1, 0, 0, 190, 0, 0, 0, 0, 5, 15, 0, 0, 0, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0, 0, 15, 15, 1, 0, 32, 32, 64, 0, 64, 64, 64, 0, 195, 0, 0, 156, 42, 0, 117, 84, 0, 78, 126, 0, 39, 168, 0, 0, 210, 0, 0, 168, 45, 0, 126, 90, 0, 84, 135, 0, 42, 180, 0, 0, 225, 39, 0, 180, 78, 0, 135, 117, 0, 90, 156, 0, 45, 0, 0, 0, 65, 0, 0, 52, 14, 0, 39, 28, 0, 26, 42, 0, 13, 56, 0, 0, 70, 0, 0, 56, 15, 0, 42, 30, 0, 28, 45, 0, 14, 60, 0, 0, 75, 13, 0, 60, 26, 0, 45, 39, 0, 30, 52, 0, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

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

byte header[1] = {HEADER};
byte crc[1] = {CRC8(dataContent, SIZE_DATA)};

//output data on led
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

void setup() {
  pinMode(PIN_LED_BLUE, OUTPUT);
  digitalWrite(PIN_LED_BLUE, LOW);
}

void loop() {
  broadcastData(header, 1);
  broadcastData(dataContent, SIZE_DATA);
  broadcastData(crc, 1);
}
