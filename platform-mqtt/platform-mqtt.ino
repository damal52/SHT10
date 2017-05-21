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

char printbuf[100];
int arrivedcount = 0;
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

char* deviceID        = "mech_demo";
const char* pub_topic = "temperature";
const char* sub_topic = "temperature";

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONNECTION 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void connect()
{
  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
  
  char hostname[]       = "abodecity.com";
  int port              = 1883;  
  data.MQTTVersion      = 3;
  data.clientID.cstring = deviceID;
  
  sprintf(printbuf, "Connecting to %s:%d\n", hostname, port);
  Serial.print(printbuf);
  
  int rc  = ipstack.connect(hostname, port);
  if (rc != 1)
  {
    sprintf(printbuf, "rc from TCP connect is %d\n", rc);
    Serial.print(printbuf);
  }
 
  Serial.println("MQTT connecting");  
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
  int intPart   = int(val);
  long decPart  = 100*(val-intPart);
                                  
  if(decPart>0)return(decPart);
  else if(decPart<0)return((-1)*decPart);
  else if(decPart=0)return(00);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CHECK SENSORS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Data getData(){
  setupSHT10();
  struct Data data;
  data    = SHT10();
  // data.t  = random(20,40);
  // data.h  = random(2,10);
  return data;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SEND MQTT MESSAGE
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool sendData(Data data){
  MQTT::Message message;

  // DEVICE
  String device = String(deviceID);

  // TEMPERATURE
  String stem = String(int(data.t))+ "."+String(getDecimal(data.t));
  char ctem[stem.length()+1];
  stem.toCharArray(ctem,stem.length()+1);
  
  // HUMIDITY
  String shum = String(int(data.h))+ "."+String(getDecimal(data.h));
  char chum[shum.length()+1];
  shum.toCharArray(chum,shum.length()+1);

  String suf = "{\"device\":\""+device+"\",\"temperature\":\""+stem+"\",\"humidity\":\""+shum+"\"}";
 
  char buf[suf.length()];
  sprintf(buf,"{\"device\":\"%s\",\"temperature\":\"%s\",\"humidity\":\"%s\"}",deviceID,ctem,chum);

  Serial.println("Temperature:"+stem+" Humidity:"+shum);

  message.qos         = MQTT::QOS0;
  message.retained    = false;
  message.dup         = false;
  message.payload     = (void*)buf;
  message.payloadlen  = suf.length();
  
  int rc = client.publish(pub_topic, message);
  while (arrivedcount == 1)
    client.yield(1000);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MAIN LOOP
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned long ms    = millis();
unsigned long tmill = millis();
unsigned long bmill = millis();

void loop()
{
  if(!client.isConnected())connect();

	// SEND A SENSOR READING EVERY 3 SECONDS
	if(ms-tmill > 5000){
		sendData(getData());
		tmill = ms;
	}

	// SEND A BATTERY MESSAGE EVERY 10 SECONDS
	if(ms-bmill > 60000){
		Serial.println("BATTERY READING");
		bmill = ms;
	}
	
	//CONTINUALLY INCREMENT MS WITH CURRENT SYSTEM MILLISECONDS
	ms = millis();
}

