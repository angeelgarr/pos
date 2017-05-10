#include<SoftwareSerial.h>
#define TxD 3
#define RxD 2
#define LED_PIN 13

SoftwareSerial bluetoothSerial(TxD,RxD);

void setup()
{
  bluetoothSerial.begin(9600);
  Serial.begin(9600);
  pinMode(LED_PIN,OUTPUT);
}
char c;
void loop()
{
  if(bluetoothSerial.available())
  {
    c = bluetoothSerial.read();
    Serial.println(c);
    if(c == '1')
    {
      digitalWrite(LED_PIN,HIGH);
    }
    if(c == '0')
    {
      digitalWrite(LED_PIN,LOW);
    }
  }
}
