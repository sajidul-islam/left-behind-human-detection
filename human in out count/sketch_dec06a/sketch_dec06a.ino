const int sensor1Pin = 2;   
const int sensor2Pin = 3;   
int count = 0;              
unsigned long sensorTime = 0;  

bool sensor1crossed = false;
bool sensor2crossed = false;

void setup() {
  pinMode(sensor1Pin, INPUT_PULLUP);
  pinMode(sensor2Pin, INPUT_PULLUP);
  Serial.begin(9600); 
}

void loop() {
  
  walkin();
  

}

void walkin()
{
  int sensor1State = digitalRead(sensor1Pin);
  int sensor2State = digitalRead(sensor2Pin);

  if (sensor1State == LOW && sensor1crossed == false) 
  {
    sensor1crossed =true;
    sensorTime = millis();
    Serial.println(sensorTime);
    Serial.println("Sensor1 crossed");
  }
  if (sensor2State == LOW && millis()- sensorTime<=1000)
  {
    count++;
    Serial.println(count);	
    delay(50);
  }
  sensor1crossed = false;
}
