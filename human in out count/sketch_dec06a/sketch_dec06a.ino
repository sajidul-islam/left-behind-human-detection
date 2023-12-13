#define DELAY_TIMEOUT 1500

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

bool condition1 = false;
bool condition2 = false;
void setup(void)
 {
   Serial.begin(9600);
   pinMode( ir_right_pin, INPUT);
   pinMode( ir_left_pin , INPUT);
 
}



void loop(void)
 {
     ir_right_state = digitalRead( ir_right_pin );
     ir_left_state =  digitalRead( ir_left_pin );
     checkWalkIn();
     checkWalkOUT();
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

