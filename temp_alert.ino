/* Temperature sensor */
#include <GSM.h>
#include <Wire.h>
#include "LowPower.h"

#define PINNUMBER ""
/* remember to include the double zero and country code before the number */
#define sendto "+48711111111"
#define precision  2
#define tmp102Address 0x48


/*delay between sleep-ups */
#define delay_time 1000

/* delay_count is multiple of 8 seconds*/
byte sleep_for_count=0;

/* define how many multiples of 8 the device will sleep for*/
#define SLEEPING_COUNT 50

/* backoff if alert has happened */
byte alerting_backoff=0;

/* initialize the library instance */
GSM gsmAccess(false); // include a 'true' parameter for debug enabled
GSM_SMS sms;
GSMPIN PINManager;
/* alert for 24,5 deg C */
int alert_temp = 245;

char floatBuffer[12];
char printBuffer[12];  
char txtMsg[165];

short restart_iterator=0;
  
/* alerting on more than condition */
byte lessmore=1;

/* alerting is by defaut on */
byte alerting_on=1;

void startGSM(){
  PINManager.begin();
  boolean notConnected = true;
  while(notConnected)
  {
    if(gsmAccess.begin(PINNUMBER)==GSM_READY){
      delay(1000);
      notConnected = false;
      Serial.println(F("GSM Connected"));
    }
    else
    {
      Serial.println(F("Not connected"));
      delay(1000);
    }
  }
}
/*
void stopGSM(){
  boolean notConnected=false;
  while(notConnected==false){
    if(gsmAccess.shutdown()){
      delay(1000);
      notConnected = true;
      Serial.println(F("GSM Stopped"));
    }
    else{
      delay(1000);
    }
  }
}
*/
void setup(){
  Serial.begin(115200);
  startGSM();
  Serial.println(F("GSM initialized")); 
  sprintf_P(txtMsg, PSTR("Sensor started. Default alert treshold!"));
  sendSMS(txtMsg);
}

void loop(){
  char received_SMS[165];
  char number[14];
  char c;
  float celsius=-100;

  if (sms.available()){
    Serial.println(F("Message received from:"));
    sms.remoteNumber(number, 14);
    Serial.println(number);
 
    // Read message bytes and print them
    int i=0;
    while(c=sms.read()){
      received_SMS[i]=c;
      i++;
    }
    parseSMS(received_SMS);
    sms.flush();
    Serial.println(F("MESSAGE DELETED"));
  }
  
  celsius = getTemperature()*10;
/*  Serial.print(F("Current temperature: "));
  Serial.println(celsius);
  Serial.print(F("alert_temp: "));
  Serial.println(alert_temp);
*/  
  if (alerting_backoff==0){
      if (alerting_on==1){
        if (lessmore==0){
          if (celsius < alert_temp){
            dtostrf(celsius, precision+3, precision, floatBuffer);
            sprintf(printBuffer, "%s", floatBuffer);
            sprintf_P(txtMsg, PSTR("temp below treshold %d /10. CurrentTemp=%s /10"), alert_temp, printBuffer);
            Serial.println(txtMsg);
            sendSMS(txtMsg);
            sleep_for_count=SLEEPING_COUNT;
            alerting_backoff=1;
          }
        }
       
       if (lessmore==1){
          if (celsius > alert_temp){
            dtostrf(celsius, precision+3, precision, floatBuffer);
            sprintf(printBuffer, "%s", floatBuffer);
            sprintf_P(txtMsg, PSTR("temp above treshold %d /10. CurrentTemp=%s /10"), alert_temp, printBuffer);
            Serial.println(txtMsg);
            sendSMS(txtMsg);
            sleep_for_count=SLEEPING_COUNT;
            alerting_backoff=1;
          }
        }
      }
  }
  

  if(sleep_for_count > 0)
  {
      LowPower.idle(SLEEP_8S, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_ON, USART0_OFF, TWI_OFF);
      sleep_for_count--;
      if(sleep_for_count==1){
        alerting_backoff=0;
      }
  }
 
  int reg = PINManager.checkReg(); 
  if(reg != 0 && reg != 1){
        startGSM();
        delay(1000);
   }
  
  delay(delay_time); //just here to slow down the measurement   
}

float getTemperature(){
  Wire.requestFrom(tmp102Address,2); 

  byte MSB = Wire.read();
  byte LSB = Wire.read();

  //it's a 12bit int, using two's compliment for negative
  int TemperatureSum = ((MSB << 8) | LSB) >> 4; 

  float celsius = TemperatureSum*0.0625;
  return celsius;
}

void sendSMS(char *txtMsg){

  Serial.print(F("Message to mobile number: "));
  Serial.println(sendto);

  // sms text
  Serial.println(F("SENDING"));
  Serial.println();
  Serial.println(F("Message:"));
  Serial.println(txtMsg);

  // send the message
  sms.beginSMS(sendto);
  sms.print(txtMsg);
  sms.endSMS(); 
  Serial.println(F("\nCOMPLETE!\n"));  
  delay(7000);
}


int parseSMS(char* received_SMS){
 
  char buffer[4];

  if (strncmp("A",received_SMS,1)==0){
      if(received_SMS[3] >= 0x30 && received_SMS[3] <= 0x39 && received_SMS[4] >= 0x30 && received_SMS[4] <= 0x39 && received_SMS[5] >= 0x30 & received_SMS[5] <= 0x39){
        strncpy(buffer, &received_SMS[3], 3);
        buffer[3]='\0';
        alert_temp=atoi(buffer);
        if(received_SMS[2]=='-') {
          alert_temp=-alert_temp;
        }  
       } 
      else return 0;
      
      if(received_SMS[1]=='l'){
        // to alert on drop below treshold
        lessmore=0;
        alerting_on=1;
        sprintf_P(txtMsg, PSTR("ACK. I will alert for t < %d/10"), alert_temp);
        Serial.println(txtMsg);
        sendSMS(txtMsg);
      }
      else if(received_SMS[1]=='m'){
        lessmore=1;
        alerting_on=1;
        sprintf_P(txtMsg, PSTR("ACK. I will alert for t > %d/10"), alert_temp);
        Serial.println(txtMsg);
        sendSMS(txtMsg);
      }
      else return 0;
  }
  else if(received_SMS[0]=='T'){
      if(alerting_on==0)
        sprintf_P(txtMsg, PSTR("Alerting is off. CurrentTemp=%d /10"), (int)(getTemperature()*10));
      else 
        sprintf_P(txtMsg, PSTR("CurrentTemp=%d /10, alert set for %d /10"), (int)(getTemperature()*10), alert_temp);
      Serial.println(txtMsg);
      sendSMS(txtMsg);  
  }
  /*
  else if(strncmp("Off",received_SMS,3)==0){
      sprintf_P(txtMsg, PSTR("Alerting switched off"));
      Serial.println(txtMsg);
      sendSMS(txtMsg);  
      alerting_on=0;
  }
  */
  else return 0;
 
  return 1;
}
 
