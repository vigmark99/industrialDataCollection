////////////////////////////////////////////////////////////////////////////////////////////
#include "ESP8266WiFi.h"
#include "PubSubClient.h"
////////////////////////////////////////////////////////////////////////////////////////////
//const char* ssid = "i40tk"; //Enter SSID
//const char* password = "PbKbTtKa5"; //Enter Password
const char* ssid = "lkedves-home"; //Enter SSID
const char* password = "takuan!S0h0"; //Enter Password

//const char *mqtt_broker = "192.168.33.211"; //MQTT 
//const int mqtt_port = 1883; //MQTT port
const char *mqtt_broker = "152.66.34.82"; //MQTT 
const int mqtt_port = 1883; //MQTT port
const char* user = "root";
const char* pass = "kiskacsa";

WiFiClient espClient;
PubSubClient client(espClient);

////////////////////////////////////////////////////////////////////////////////////////////
const byte interruptPin = 4;
volatile float cnt = 0;

volatile float fluctuationAverage[6];
volatile double timestamp = 0;
volatile int numberOfHoles = 6;
volatile int holeIndex = 0;

volatile int Counts[100];
volatile int  indexOfCounts = 0;

volatile bool Writing = false;
////////////////////////////////////////////////////////////////////////////////////////////
void ICACHE_RAM_ATTR add() {
  cnt++;
  
  float current = millis();
  float timeSpent = current - timestamp;
  timestamp = current;
  
  fluctuationAverage[holeIndex] = (fluctuationAverage[holeIndex] + timeSpent) / 2;
  if(holeIndex == 5 ){
    holeIndex = 0;
  }
  else{
    holeIndex++;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
     delay(500);
     Serial.print("*");
  }

  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);

  //MQTT CONNECT
  while (!client.connected()) {
    Serial.println("Connecting to public emqx mqtt broker.....");
    if (client.connect("esp8266-client",user,pass)) {
        Serial.println("Public emqx mqtt broker connected");
    } else {
        Serial.print("failed with state ");
        Serial.print(client.state());
        delay(2000);
    }
  }
  
  pinMode(interruptPin, INPUT);
  attachInterrupt(interruptPin, add, RISING);

}

////////////////////////////////////////////////////////////////////////////////////////////
void callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    for (int i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
    }
    Serial.println();
    Serial.println("-----------------------");
}

////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  client.loop();

  delay(10);
  Counts[indexOfCounts] = cnt;
  cnt = 0;

  if(indexOfCounts >= 99){
    Writing = true;
    indexOfCounts = 0;
  }
  else{
    indexOfCounts++;
  }

  //Az RPM kiszamolasa es kiirasa
  if(Writing){
    float sumOfCounts = 0; 
    for(int i = 0; i < 100 ; i++){
      sumOfCounts += Counts[i];
    }
    float numberOfRounds = sumOfCounts / numberOfHoles;
    float rpm  = numberOfRounds * 60;


    char message[100];
    sprintf(message,"eszterga, rpm=%f", rpm);
    client.publish("esp8266/rpm", message);

    sprintf(message,"eszterga, hole1=%f", fluctuationAverage[0]);
    client.publish("esp8266/hole1", message);

    sprintf(message,"eszterga, hole2=%f", fluctuationAverage[1]);
    client.publish("esp8266/hole2", message);

    sprintf(message,"eszterga, hole3=%f", fluctuationAverage[2]);
    client.publish("esp8266/hole3", message);

    sprintf(message,"eszterga, hole4=%f", fluctuationAverage[3]);
    client.publish("esp8266/hole4", message);

    sprintf(message,"eszterga, hole5=%f", fluctuationAverage[4]);
    client.publish("esp8266/hole5", message);

    sprintf(message,"eszterga, hole6=%f", fluctuationAverage[5]);
    client.publish("esp8266/hole6", message);

    Writing = false;
  }
}
