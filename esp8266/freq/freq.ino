/*
 Basic ESP8266 MQTT example
 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.
 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off
 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.
 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"
*/

#include <arduinoFFT.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.

const char* ssid = "Pocok";
const char* password = "555444666";
const char* mqtt_server = "152.66.34.82";
//const char* mqtt_server = "192.168.33.212";
const uint16_t samples = 1024; //This value MUST ALWAYS be a power of 2
const double samplingFrequency = 10000; //Hz, must be less than 10000 due to ADC

WiFiClient espClient;
PubSubClient client(espClient);
#define MSG_BUFFER_SIZE	(400)
char msg[MSG_BUFFER_SIZE];

arduinoFFT FFT = arduinoFFT();
#define CHANNEL A0

unsigned int sampling_period_us;
unsigned long microseconds;
double vReal[samples];
double vImag[samples];
//double dominant;

#define SCL_INDEX 0x00
#define SCL_TIME 0x01
#define SCL_FREQUENCY 0x02
#define SCL_PLOT 0x03

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  //Serial.println();
  //Serial.print("Connecting to ");
  //Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //Serial.print(".");
  }

  randomSeed(micros());

  //Serial.println("");
  //Serial.println("WiFi connected");
  //Serial.println("IP address: ");
  //Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    //Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      //Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("outTopic", "hello world");
      // ... and resubscribe
    } else {
      //Serial.print("failed, rc=");
      //Serial.print(client.state());
      //Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  sampling_period_us = round(1000000*(1.0/samplingFrequency));
  Serial.begin(115200);
  while(!Serial);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  /*SAMPLING*/
  microseconds = micros();
  
  for(int i=0; i<samples; i++)
  {
      vReal[i] = analogRead(CHANNEL);
      vImag[i] = 0;
      while(micros() - microseconds < sampling_period_us){
        //empty loop
      }
      microseconds += sampling_period_us;
  }
  
  /* Print the results of the sampling according to time */
  FFT.Windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD); /* Weigh data */
  FFT.Compute(vReal, vImag, samples, FFT_FORWARD); /* Compute FFT */
  FFT.ComplexToMagnitude(vReal, vImag, samples); /* Compute magnitudes */
  double maxi[] = {0,0,0,0,0,0,0,0,0,0};
  int l;
  for( int i = 2; i < (samples/2); i++)
  {
    for( int j=0; j<10; j++)
    {
      l=maxi[j];
      if(vReal[i] > vReal[l] || l==0) 
      {
        for(int k=9;k>j;k--)
        {
          maxi[k]=maxi[k-1];
        }
        maxi[j]=i;
        break;
      }
    }
  }
  int j;
  //dominant = maxi/4;
  sprintf(msg, "eszterga");
  for( int i = 0; i < 10; i++)
  {
    //if(vReal[i] >= dominant) 
    //{
      //Serial.println((samplingFrequency/samples)*i);
      //Serial.print(" Hz -> ");
      //Serial.println(vReal[i],4); 
    j=maxi[i];
    snprintf (msg, MSG_BUFFER_SIZE, "%s, freq=%.2f, amp=%.4f", msg, (samplingFrequency/samples)*maxi[i], vReal[j]);
    //}
  }
  Serial.println(msg);
  client.publish("esp8266/freq", msg);
  delay(5000);
  
}
