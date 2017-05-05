// This version of code was confirmed to work on the mechintelligent hardware 2017.04.26
// update 04.26: temp and humidity struct is sent successfully from SHT10 function
// -D. Malawey


/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution. 
 *
 * The Eclipse Public License is available at 
 *   http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at 
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial contribution
 *******************************************************************************/

//#define MQTTCLIENT_QOS2 1  //DPM 2107.04.15 commented out to resolve compile error

#include <SPI.h>
#include <WiFi.h>
#include <WifiIPStack.h>
#include <Countdown.h>
#include <MQTTClient.h>

//-------------GLOBAL VARIABLES----------------------//
struct Data {
  float t;
  float h;
};

// your network name also called SSID
char ssid[] = "MOTOROLA-CA654";  //DustinLadd wifi 
// "Maker"; //M&M wifi name
//"DANGER_2.4";
// your network password
char password[] = "e0aa4d228a7fd5d71143"; //DustinLadd wifi password
// "3900BotClub"; // M&M wifi password
// "regnad41";  // DPM wifi password

char printbuf[100];

int arrivedcount = 0;

//-------------SECTION ONLY USED FOR SUBSCRIBING--------------------------//
void messageArrived(MQTT::MessageData& md)
{
  MQTT::Message &message = md.message;
  
  sprintf(printbuf, "Message %d arrived: qos %d, retained %d, dup %d, packetid %d\n", 
		++arrivedcount, message.qos, message.retained, message.dup, message.id);
  Serial.print(printbuf);
  sprintf(printbuf, "Payload %s\n", (char*)message.payload);
  Serial.print(printbuf);
}


WifiIPStack ipstack;
MQTT::Client<WifiIPStack, Countdown> client = MQTT::Client<WifiIPStack, Countdown>(ipstack);

const char* pub_topic = "device_state";  //DPM publishing topic
const char* sub_topic = "mechintelligent/request0"; //DPM 04.24 subscribing topic

void connect()
{
  char hostname[] = "abodecity.com"; //"broker.hivemq.com";
  int port = 1883;
  sprintf(printbuf, "Connecting to %s:%d\n", hostname, port);
  Serial.print(printbuf);
  int rc = ipstack.connect(hostname, port);
  if (rc != 1)
  {
    sprintf(printbuf, "rc from TCP connect is %d\n", rc);
    Serial.print(printbuf);
  }
 
  Serial.println("MQTT connecting");
  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;       
  data.MQTTVersion = 3;
  data.clientID.cstring = (char*)"mechintelligent";
  rc = client.connect(data);
  if (rc != 0)
  {
    sprintf(printbuf, "rc from MQTT connect is %d\n", rc);
    Serial.print(printbuf);
  }
  Serial.println("MQTT connected");

// ----------------------RECEIVING PACKETS--------------------------------//
  rc = client.subscribe(sub_topic, MQTT::QOS2, messageArrived);   
  if (rc != 0)
  {
    sprintf(printbuf, "rc from MQTT subscribe is %d\n", rc);
    Serial.print(printbuf);
  }
  Serial.println("MQTT subscribed");
}


//----------------------SETUP---------------------------------------------//
void setup()
{
  Serial.begin(115200);
  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to Network named: ");
  // print the network name (SSID);
  Serial.println(ssid); 
  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED) {
    // print dots while we wait to connect
    Serial.print(".");
    delay(300);
  }
  
  Serial.println("\nYou're connected to the network");
  Serial.println("Waiting for an ip address");
  
  while (WiFi.localIP() == INADDR_NONE) {
    // print dots while we wait for an ip addresss
    Serial.print(".");
    delay(300);
  }

  Serial.println("\nIP Address obtained");
  // We are connected and have an IP address.
  Serial.println(WiFi.localIP());
  connect();
}


//----------------------MAIN LOOP---------------------------//
void loop()
{
  if (!client.isConnected())
    connect();
  
  MQTT::Message message;
  
  arrivedcount = 0;
  char buf[100];
  char buf2[100];

  setupSHT10();           // call function to setup sensor
  struct Data data;
  data = SHT10();            // call the function which returns a Data struct


  //String datastring = "{\"stateKey\":\"temperature\",\"temp\":"+data.t+"}";
  //int stringSize = length(datastring);
  // Send and receive QoS 1 message
  //{"stateKey": "temperature", "user":1, "batch":1, "target":1, "device":1, "data":70}
  //sprintf(buf2, "{\"stateKey\":\"temperature\"}");
  //sprintf(buf, "temp,%d,humid,%d", data.t,data.h);
  Serial.println(data.t); //change back to buf after experiment.
  message.qos = MQTT::QOS1;
  message.retained=false;
  message.dup = false;
  message.payload = (void*)buf2;
  message.payloadlen = strlen(buf)+1;
  int rc = client.publish(pub_topic, message);
  while (arrivedcount == 1)
    client.yield(1000);
  
  delay(3000);
}

//-----------------UNUSED SECTIONS------------------------//

//  // Send and receive QoS 2 message
//  sprintf(buf, "Hello World!  QoS 2 message");
//  Serial.println(buf);
//  message.qos = MQTT::QOS2;
//  message.payloadlen = strlen(buf)+1;
//  rc = client.publish(topic, message);
//  while (arrivedcount == 2)
//    client.yield(1000);


//  // Send and receive QoS 0 message
//  sprintf(buf, "Hello World! QoS 0 message");
//  Serial.println(buf);
//  message.qos = MQTT::QOS0;
//  message.retained = false;
//  message.dup = false;
//  message.payload = (void*)buf;
//  message.payloadlen = strlen(buf)+1;
//  int rc = client.publish(topic, message);
//  while (arrivedcount == 0)
//    client.yield(1000);
