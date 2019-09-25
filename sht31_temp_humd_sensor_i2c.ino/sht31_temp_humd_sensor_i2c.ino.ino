// Modified by Michael 

// Master requests to read register 1 or 2.
//
// ver 1 - send status read command and read status registers
// ver 2 - make a command as an input parameter to the function.
// ver 3 - print the status reg value and CRC in hex to the term.
// ver 4 - make a command as an input parameter to the function.
// ver 5 - make status read length.
// ver 6 - add more commands
// ver 7 - clear status 
//         print the status of write transmission (success or NACK)
// ver 9 - make subroutine printdata
// ver 10 - get temperature/humidity
// ver 11 - remove status command
// ver 12 - For NACK, available = 0.  No bytes will be read.   Use that 
//          to check when the measurement is done.
// ver 13 - remove some serial.print (too much delay)
// ver 14 - remove the big delay to see multiple temperature read.
// ver 15 - add 


#include <Wire.h>

const byte sensor_addr = 0x44;
const word command_statusread  = 0xF32D;  // read register
const word command_statusclear = 0x3041;  // clear register
const word command_softreset   = 0x30A2;  // generate a reset
const word command_heaton      = 0x306D;  // enable heater
const word command_heatoff     = 0x3066;  // disable heater
const word command_startmeasL  = 0x2416;  // Start single shot measurement with 4ms sensing duration
const word command_startmeasM  = 0x240B;  // Start single shot measurement with 4ms sensing duration
const word command_startmeasH  = 0x2400;  // Start single shot measurement with 4ms sensing duration
const word command_break       = 0x3093;  // Break the current sensing, so another new command can be issued.

word temperature_raw;
word humidity_raw;
float temperature;
float humidity;

byte status_reg_len = 3;  //including crc

byte write2slave (int addr, word command);
bool readslave (int addr, int len);
void printdata (int len);

byte datain[6];   // global array (for now)
byte transtatus;  // transmission status
bool flag;        // status flag for reading measurment data

void setup() {
  Wire.begin(); // join i2c bus (address optional for master)

  Serial.begin(9600);           // start serial for output
  Serial.println ("Start sending commands");
}

void loop() {
  // Start measurement
  Serial.print   ("I2C write command:");
  Serial.println (command_startmeasH,HEX);
  //Serial.print   (" I2C write status:"); 
  transtatus = write2slave (sensor_addr,command_startmeasH);
  //Serial.println (transtatus);
  //Serial.println (" ");

  delayMicroseconds(50);
  
  // read data
  do {
      flag = readslave(sensor_addr,6);
      if (flag) {
        Serial.println ("Pass: Correct number of bytes received.");
        Serial.print   ("Measurement Data:");
        printdata(6);

        temperature_raw = word(datain[0]) << 8;
        temperature_raw = temperature_raw + word(datain[1]);
        Serial.print   ("Temperature (raw hex value):");
        Serial.println (temperature_raw,HEX);
        temperature = 175.0 * temperature_raw / 65535.0;
        temperature = temperature - 45;
        Serial.print   ("Temperature (deg C):");
        Serial.println (temperature);
        
        humidity_raw    = word(datain[3]) << 8;
        humidity_raw    = humidity_raw + word(datain[4]);
        Serial.print   ("Humidity (raw hex value):");
        Serial.println (humidity_raw,HEX);    
        humidity = 100.0 * humidity_raw / 65535.0;
        Serial.print   ("Humidity (percent):");
        Serial.println (humidity);
            
        Serial.println (" ");
      }
      //} else {
      //  Serial.println ("Error: Not correct number of bytes received.");
      //}
      delayMicroseconds(50);
  } while (!flag);

  //delay(500);
}

// return the status of the transmission
byte write2slave (int addr, word command) {
  byte out;
  Wire.beginTransmission(addr); // transmit to device addr
  if (command) {
    out = byte((command >> 8) & 0x00FF); 
    Wire.write(out);        // sends 1 bytes (MSB)
    out = byte(command & 0x00FF); 
    Wire.write(out);        // sends 1 bytes (LSB)
  }
  return Wire.endTransmission();    // stop transmitting
}

// Read a string of len from the slave.  Return error flag.  
// If error flag = true, the number of received bytes is correct.  
bool readslave (int addr, int len) {

  Wire.requestFrom(addr, len);    // request 6 bytes from slave device #8

  int index = 0;
  byte x    = 0;
  while (Wire.available()) { // slave may send less than requested
    x = Wire.read(); // receive a byte as character
    datain[index++] = x;    
  }
  if (index == len) {
    return true;
  } else {
    return false;
  }
}

void printdata (int len) {
  for (int index=0; index < len; index++) {
      if (datain[index] == 0) {
        Serial.print   ("00");
      } else {
        Serial.print   (datain[index],HEX);
      }
  }
  Serial.println (" end");
  return;
}
