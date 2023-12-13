#include <ArduinoBLE.h>
#include <WiFi.h>
#include <PubSubClient.h>


#define DELAY_TIMEOUT 1500

//Wifi
const char* SSID = "y9";
const char* PWD = "12345678";


WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient); 
//MQTT
char *mqttServer = "broker.hivemq.com";
int mqttPort = 1883;

//human count

int ir_right_pin = 14;
int ir_left_pin = 27;

int ir_right_state = 0;
int ir_left_state  = 0;



int ir_right_state_last = -1;
int ir_left_state_last  = -1;



int in_counter = 0;
int out_counter = 0;



bool bWalkIn = false;
bool bWalkOut = false;

unsigned long tm;


unsigned long left_human_update_time = 0;

int a  = 0;
int seat_sensor = 0;

bool seat_occupied = false;

void connectToWiFi() {
  Serial.print("Connectiog to ");
 
  WiFi.begin(SSID, PWD);
  Serial.println(SSID);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.print("Connected.");
  
} 

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Callback - ");
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
}

void setupMQTT() {
  mqttClient.setServer(mqttServer, mqttPort);
  // set the callback function
  //mqttClient.setCallback(callback);
}

void reconnect() {
  Serial.println("Connecting to MQTT Broker...");
  while (!mqttClient.connected()) {
      Serial.println("Reconnecting to MQTT Broker..");
      String clientId = "ESP32Client-";
      clientId += String(random(0xffff), HEX);
      
      if (mqttClient.connect(clientId.c_str())) {
        Serial.println("Connected.");
        // subscribe to topic
        mqttClient.subscribe("/hello/text");
      }
      
  }
}


void setup() 
{
  Serial.begin(9600);
  connectToWiFi();
  setupMQTT();

  if (!BLE.begin()) 
  {
    while (1);
  }


  BLE.scan();

  pinMode( ir_right_pin, INPUT);
  pinMode( ir_left_pin , INPUT);

}




void bluetooth()
{
  BLEDevice peripheral = BLE.available();

    if (peripheral.localName() == "Node1")
     {
        peripheral.address();
        peripheral.localName();
        peripheral.advertisedServiceUuid();
        BLE.stopScan();
        monitorSensorTagButtons(peripheral);
        BLE.scan();
    }
}

void monitorSensorTagButtons(BLEDevice peripheral) 
{

  peripheral.connect(); 
  
  if (peripheral.discoverService("19B10000-E8F2-537E-4F6C-D104768A1215")) 
  {
    Serial.println("Service discovered");
  } 
  else 
  {
    Serial.println("Attribute discovery failed.");
    peripheral.disconnect();

    //while (1);
    return;
  }

  // retrieve the simple key characteristic
  BLECharacteristic simpleKeyCharacteristic = peripheral.characteristic("19B10001-E8F2-537E-4F6C-D104768A1216");

  // subscribe to the simple key characteristic
  Serial.println("Subscribing to simple key characteristic ...");
  if (!simpleKeyCharacteristic) {
    Serial.println("no simple key characteristic found!");
    peripheral.disconnect();
    return;
  } else if (!simpleKeyCharacteristic.canSubscribe()) {
    Serial.println("simple key characteristic is not subscribable!");
    peripheral.disconnect();
    return;
  } else if (!simpleKeyCharacteristic.subscribe()) {
    Serial.println("subscription failed!");
    peripheral.disconnect();
    return;
  } else {
    Serial.println("Subscribed");
    Serial.println("Press the right and left buttons on your SensorTag.");
  }
 
  while (peripheral.connected() ) {


    // check if the value of the simple key characteristic has been updated
    if (simpleKeyCharacteristic.valueUpdated()) {
      // yes, get the value, characteristic is 1 byte so use byte value
      byte value = 0;
      
      simpleKeyCharacteristic.readValue(value);
 
      if(value == 72)
      {
        mqttClient.loop();
        mqttClient.publish("/hello/text", "Device Id Gateway1: HumanDetected");
        Serial.println("Published H");

      }
      if (value == 78)
      {
        mqttClient.loop();
        mqttClient.publish("/hello/text", "Device Id Gateway1: NoHumanDetected");
        Serial.println("Published N");

      }
    }
  }

  Serial.println("SensorTag disconnected!");
}


void checkWalkIn()
{
    if( ir_right_state != ir_right_state_last )
    {
         ir_right_state_last = ir_right_state;

         if( (bWalkIn == false) && ( ir_right_state == LOW ) )
         {
              bWalkIn = true;
              tm = millis();
         }

     }
     if( (millis() - tm) > DELAY_TIMEOUT )
     {
          bWalkIn = false;
     }
     if( bWalkIn && (ir_left_state == LOW) && (ir_right_state == HIGH) ){

          bWalkIn = false;
          in_counter++;
          Serial.print("In person");
          Serial.println(in_counter);
     }
}

void checkWalkOUT()
{
    if( ir_left_state != ir_left_state_last )
    {
         ir_left_state_last = ir_left_state;

         if( (bWalkOut == false) && ( ir_left_state == LOW ) )
         {
              bWalkOut = true;
              tm = millis();
         }
     }

     if( (millis() - tm) > DELAY_TIMEOUT )
     {
          bWalkOut = false;
     }
     if( bWalkOut && (ir_right_state == LOW) && (ir_left_state == HIGH) )
     {
          bWalkOut = false;
          out_counter++;
          Serial.print("Out person");
          Serial.println(out_counter);
     }
}

void occupency_mqtt()
{
    checkWalkIn();
    checkWalkOUT();

    a  = in_counter - out_counter;
    seat_sensor = analogRead(34);
    if(seat_sensor<200)
    {
      seat_occupied = true;
      if (millis() - left_human_update_time > 5000) 
     {
        String message = "Device ID:Gateway1,number of person in the bus:  " + String(a)+"  "+"Driver Seat Occupied:  "+ String(seat_occupied);
        mqttClient.publish("/hello/text",message.c_str());
        Serial.println("Published Number of person in the bus & Driver Sit");
        Serial.println(a);

        left_human_update_time = millis();
      }
    }
    if(seat_sensor>200)
    {
      seat_occupied = false;
    
    }

  if (millis() - left_human_update_time > 5000) 
     {
        String message = "Device ID:Gateway1,number of person in the bus:  " + String(a)+"  "+"Driver Seat Occupied:  "+ String(seat_occupied);
        mqttClient.publish("/hello/text",message.c_str());
        Serial.println("Published Number of person in the bus & Driver Sit");
        left_human_update_time = millis();
      }
}

void loop() 
{

    ir_right_state = digitalRead( ir_right_pin );
    ir_left_state =  digitalRead( ir_left_pin );

    if (!mqttClient.connected())
        reconnect();
    
    occupency_mqtt();
    
    bluetooth();



    
}


