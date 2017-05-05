// Function return code definitions
const uint8_t S_Err_NoACK  = 1;  // ACK expected but not received
const uint8_t S_Err_CRC    = 2;  // CRC failure
const uint8_t S_Err_TO     = 3;  // Timeout
const uint8_t S_Meas_Rdy   = 4;  // Measurement ready
// Sensirion command definitions:      adr command r/w
const uint8_t MEAS_TEMP   = 0x03;   // 000  0001   1
const uint8_t MEAS_HUMI   = 0x05;   // 000  0010   1
const uint8_t STAT_REG_W  = 0x06;   // 000  0011   0
const uint8_t STAT_REG_R  = 0x07;   // 000  0011   1
const uint8_t SOFT_RESET  = 0x1e;   // 000  1111   0
// Status register writable bits
const uint8_t SR_MASK     = 0x07;
// getByte flags
const bool noACK  = false;
const bool ACK    = true;
// Temperature & humidity equation constants
  const float D1  = -39.6;          // for deg C @ 3.3V
  const float D2h =   0.01;         // for deg C, 14-bit precision
  const float D2l =   0.04;         // for deg C, 12-bit precision
  const float C1  = -2.0468;        // for V4 sensors
  const float C2h =  0.0367;        // for V4 sensors, 12-bit precision
  const float C3h = -1.5955E-6;     // for V4 sensors, 12-bit precision
  const float C2l =  0.5872;        // for V4 sensors, 8-bit precision
  const float C3l = -4.0845E-4;     // for V4 sensors, 8-bit precision
  const float T1  =  0.01;          // for V3 and V4 sensors
  const float T2h =  0.00008;       // for V3 and V4 sensors, 12-bit precision
  const float T2l =  0.00128;       // for V3 and V4 sensors, 8-bit precision

//Temperature & humidity variables
  float temp;
  float hum;
  uint16_t result;
  
#define PULSE_LONG  delay(3)
#define PULSE_SHORT delay(1)

#define _pinData0 P6_0
#define _pinData1 P6_1
#define _pinData2 P6_2
#define _pinData3 P6_3

#define _pinClock0 P6_4
#define _pinClock1 P7_0
#define _pinClock2 P3_6
#define _pinClock3 P3_5

#define LED RED_LED
// INITIAL SETUP ///////////////////////////////////////////////////////////////////////////////////////////////////
void setupSHT10() {   // DPM changed from setup to setupSHT10
  // Serial.begin(115200); // DPM commented, serial begin takes place in main setup              
  pinMode(LED, OUTPUT);  

  pinMode(_pinClock0, OUTPUT);  
  pinMode(_pinClock1, OUTPUT);
  pinMode(_pinClock2, OUTPUT);
  pinMode(_pinClock3, OUTPUT);

  pinMode(_pinData0, OUTPUT);
  pinMode(_pinData1, OUTPUT);
  pinMode(_pinData2, OUTPUT);
  pinMode(_pinData3, OUTPUT); 
}
// LOOP BEGIN ///////////////////////////////////////////////////////////////////////////////////////////////////
struct Data SHT10() {   //DPM changed from loop to SHT10
Serial.println("");
  Serial.println("Sensor 0");
resetConnection0();
delay(100);
startTransmission0();
PULSE_LONG;
putByte0(MEAS_TEMP);
    
  digitalWrite(LED, HIGH);  
  delay(1000); 
  Serial.print("Temperature: ");
  result = getByte0(noACK);
  temp = (result*D2h+D1); // Equation for temperature
  Serial.println(temp);
  digitalWrite(LED, LOW);    
  delay(1000);      

startTransmission0();
PULSE_LONG;
putByte0(MEAS_HUMI);
    
  digitalWrite(LED, HIGH);
  delay(1000);
  Serial.print("Humidity: ");
  result = getByte0(noACK);
  hum = (C1 + C2h*result +C3h*result*result); // Equation for humidity
  Serial.println(hum);
  digitalWrite(LED, LOW);
  delay(1000); 

//  int tempInt = int(temp);
//  int tempTenth = ((temp-tempInt)*100);  // DPM make an int from temperature decimal places
//  int humInt = int(hum);
//  int humTenth = ((hum-humInt)*100);  // DPM make an int from temperature decimal places



//  struct Data sht10_func() {  // this function is named above
  struct Data d;
  d.t = temp;
  d.h = hum;
  return d;
}
