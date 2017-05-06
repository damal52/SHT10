#include <SPI.h>
#include <WiFi.h>
#include <WifiIPStack.h>
#include <Countdown.h>
#include <MQTTClient.h>

// CREATE GLOBAL SPACE
struct Data {
  float t,h;
};

// NETWORK
char ssid[]     = "MOTOROLA-CA654";
char password[] = "e0aa4d228a7fd5d71143";

int arrivedcount = 0;
char printbuf[100];

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SUBSCRIBING 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void messageArrived(MQTT::MessageData& md)
{
  MQTT::Message &message = md.message;  
  sprintf(printbuf, "Message %d arrived: qos %d, retained %d, dup %d, packetid %d\n",++arrivedcount, message.qos, message.retained, message.dup, message.id);
  Serial.print(printbuf);
  sprintf(printbuf, "Payload %s\n", (char*)message.payload);
  Serial.print(printbuf);
}

WifiIPStack ipstack;
MQTT::Client<WifiIPStack, Countdown> client = MQTT::Client<WifiIPStack, Countdown>(ipstack);

const char* pub_topic = "device_state";                     //DPM publishing topic
const char* sub_topic = "mechintelligent/device_state";   //DPM 04.24 subscribing topic

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONNECTION 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void connect()
{
  char hostname[] = "abodecity.com";
  int port        = 1883;
  
  sprintf(printbuf, "Connecting to %s:%d\n", hostname, port);
  Serial.print(printbuf);
  
  int rc  = ipstack.connect(hostname, port);
  if (rc != 1)
  {
    sprintf(printbuf, "rc from TCP connect is %d\n", rc);
    Serial.print(printbuf);
  }
 
  Serial.println("MQTT connecting");
  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
  data.MQTTVersion = 3;
  data.clientID.cstring = (char*)"mech_ti_04";
  rc = client.connect(data);
  if (rc != 0)
  {
    sprintf(printbuf, "rc from MQTT connect is %d\n", rc);
    Serial.print(printbuf);
  }
  Serial.println("MQTT connected");

  // RECEIVING PACKETS
  rc = client.subscribe(sub_topic, MQTT::QOS2, messageArrived);   
  if (rc != 0)
  {
    sprintf(printbuf, "rc from MQTT subscribe is %d\n", rc);
    Serial.print(printbuf);
  }
  Serial.println("MQTT subscribed");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SET UP
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup()
{
  Serial.begin(115200);
  Serial.print("Attempting to connect to Network named: ");
  Serial.println(ssid); 
  WiFi.begin(ssid,password);
  
  while ( WiFi.status() != WL_CONNECTED) {
    Serial.print(".");delay(300);
  }
  
  Serial.println("\nYou're connected to the network");
  Serial.println("Waiting for an ip address");
  
  while (WiFi.localIP() == INADDR_NONE) {
    Serial.print(".");delay(300);
  }

  Serial.println("\nIP Address obtained");
  Serial.println(WiFi.localIP());
  connect();
}

long getDecimal(float val)
{
  int intPart = int(val);
  long decPart = 100*(val-intPart); //I am multiplying by 1000 assuming that the foat values will have a maximum of 3 decimal places. 
                                    //Change to match the number of decimal places you need
  if(decPart>0)return(decPart);           //return the decimal part of float number if it is available 
  else if(decPart<0)return((-1)*decPart); //if negative, multiply by -1
  else if(decPart=0)return(00);           //return 0 if decimal part of float number is not available
}

void makeString(float f,char* chars){
dtostrf(f,4,2,chars);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MAIN LOOP
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop()
{
  if (!client.isConnected())
    connect();

  // CREATE MESSAGE OBJECT
  MQTT::Message message;
  
  arrivedcount = 0;
  setupSHT10();
  struct Data data;
  data = SHT10();

  String stem = String(int(data.t))+ "."+String(getDecimal(data.t));
  char ctem[stem.length()+1];
  stem.toCharArray(ctem,stem.length()+1);
  
  String shum = String(int(data.h))+ "."+String(getDecimal(data.h));
  char chum[shum.length()+1];
  shum.toCharArray(chum,shum.length()+1);
  
  String suf = "{\"temperature\":\""+stem+"\",\"humidity\":\""+shum+"\"}";
 
  char buf[suf.length()];
  sprintf(buf,"{\"temperature\":\"%s\",\"humidity\":\"%s\"}",ctem,chum);
  
  Serial.println(suf);
  Serial.println(buf);
  
  message.qos         = MQTT::QOS0;
  message.retained    = false;
  message.dup         = false;
  message.payload     = (void*)buf;
  message.payloadlen  = suf.length();
  
  int rc = client.publish(pub_topic, message);
  while (arrivedcount == 1)
    client.yield(1000);
  
  delay(3000);
}

