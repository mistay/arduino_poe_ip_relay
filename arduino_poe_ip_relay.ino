#include <UIPEthernet.h>
#include <EEPROM.h>
#include <TrueRandom.h>

#define LED_RED A0
#define LED_GREEN1 A1
#define LED_GREEN2 A2
#define ETH_MODULE_PON A5

#define WDTINITVALUE 10
int wdtvalue = WDTINITVALUE;

EthernetServer server = EthernetServer(23);

bool ok = false;
static uint32_t cpualivetimer = 0;

void(* resetFunc) (void) = 0;

void LEDInit() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN1, OUTPUT);
  pinMode(LED_GREEN2, OUTPUT);
}

void ETHModuleInit() {
  pinMode(ETH_MODULE_PON, OUTPUT);
  digitalWrite(ETH_MODULE_PON, LOW);
}

void LEDTest() {
  digitalWrite(LED_RED, HIGH);
  delay(250);
  digitalWrite(LED_GREEN1, HIGH);
  delay(250);
  digitalWrite(LED_GREEN2, HIGH);
  delay(250);
  digitalWrite(LED_RED, LOW);
  delay(250);
  digitalWrite(LED_GREEN1, LOW);
  delay(250);
  digitalWrite(LED_GREEN2, LOW);
}

void LEDindicateWdtLimit() {
  int i = 0;
  for (i = 0; i < 2; i++) {
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN1, HIGH);
    digitalWrite(LED_GREEN2, HIGH);

    delay(20);
    
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN1, LOW);
    digitalWrite(LED_GREEN2, LOW);
    
    delay(20);
  }
}

void LEDindicateWdtActive() {
  digitalWrite(LED_RED, HIGH);
}

void LEDindicateWdtInactive() {
  digitalWrite(LED_RED, LOW);
}

void LEDindicateReboot() {
  int i = 0;
  for (i = 0; i < 10; i++) {
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN1, HIGH);
    digitalWrite(LED_GREEN2, HIGH);

    delay(20);
    
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN1, LOW);
    digitalWrite(LED_GREEN2, LOW);
    
    delay(20);
  }
}

void LEDindicateAlive() {
  if (millis() > cpualivetimer) {
    cpualivetimer = millis() + 500;
    digitalWrite(LED_GREEN1, !digitalRead(LED_GREEN1));
  }
}

void LEDindicateActivity() {
  int i = 0;
  for (i = 0; i < 2; i++) {
    digitalWrite(LED_GREEN2, HIGH);
    delay(50);
    digitalWrite(LED_GREEN2, LOW);
    delay(50);
  }
}

void poweronEthModule() {
  digitalWrite(ETH_MODULE_PON, HIGH);
}

void setup()
{
  Serial.begin(115200);
  Serial.println("arduino_poe_ip_relay v1.2 starting...");
  
  ETHModuleInit();
  Serial.println("eth module init done.");
  
  LEDInit();
  Serial.println("led init done.");
  
  LEDTest();
  Serial.println("led test done.");
  
  poweronEthModule();
  Serial.println("powering eth module on done.");
  
  uint8_t mac[6] = { 0x06,0x02,0x03,0x04,0x05,0x06 };
  if (EEPROM.read(1) != '#') {
    Serial.println(F("\nWriting EEProm..."));
  
    EEPROM.write(1, '#');
    
    for (int i = 3; i < 6; i++) {
      EEPROM.write(i, TrueRandom.randomByte());
    }
  }
  
  // read 3 last MAC octets
  for (int i = 3; i < 6; i++) {
      mac[i] = EEPROM.read(i);
  }
  
  Serial.print("MAC: ");
  for (int i = 0; i < 6; i++) {
    Serial.print(mac[i], HEX);
        if (i<5)
          Serial.print(':');
    }
  Serial.println();
  
  
  
  
  
  Ethernet.begin(mac);
  Serial.println("ethernet init done.");

  Serial.print(F("IP: "));
  Serial.println(Ethernet.localIP());
  Serial.print(F("Subnet: "));
  Serial.println(Ethernet.subnetMask());
  Serial.print(F("Gateway: "));
  Serial.println(Ethernet.gatewayIP());
  Serial.print(F("DNS: "));
  Serial.println(Ethernet.dnsServerIP());
 
  server.begin();
  Serial.println(F("tcp/ip server started."));
}


static uint32_t timer = 0;

void handleWatchdog() {
  if (millis() > timer) {
    timer = millis() + 1000;
    Serial.print("wdt: ");
    Serial.println(wdtvalue--);

    if (wdtvalue > 0) {
      LEDindicateWdtInactive();
    }
    if (wdtvalue == 0) {
      // host gave up controlling. switching to emergeny state
      
      LEDindicateWdtLimit();
      Serial.println("wdt limit!!");
      LEDindicateWdtActive();

      int i = 2;
      for (i = 2; i < 10; i++) {

        // please note: relais are switched off by HIGH, not by LOW
        digitalWrite(i, HIGH);
      }
    }

    if (wdtvalue <= -100) {
      LEDindicateReboot();
      Serial.print("100 sec no answer from client, now restting cpu");
      resetFunc();
    }
  }
}

void loop()
{
  size_t size;

  handleWatchdog();
  LEDindicateAlive();

  if (EthernetClient client = server.available())
  {
    while ((size = client.available()) > 0) {
      handleWatchdog();
      LEDindicateAlive();

      uint8_t* msg = (uint8_t*)malloc(size);
      size = client.read(msg, size);
      Serial.print("message: ");
      Serial.write(msg, size);
      Serial.println();

      if (size >= 2) {
        int pin = msg[1] - 0x30;
        pin += 10 * (msg[0] - 0x30);

        byte command = msg[2];

        // comparison with >= instead of == cause mac os x terminal sends \r\n at the end of the line
        if (size >= 6 && command == 'P') { //PWM
          byte value = msg[5] - 0x30;
          value += 10 * (msg[4] - 0x30);
          value += 100 * (msg[3] - 0x30);
          
          pinMode(pin, OUTPUT);
          analogWrite(pin, value);
          
          ok = true;
        }
        
        if (size >= 4 && command == 'O') { //OUTPUT
          int value = msg[3] - 0x30;
          
          pinMode(pin, OUTPUT);
          digitalWrite(pin, value);

          ok = true;
        }

        if (size >= 2 && msg[2] == 'X') { // query value of a specific pin
          //byte value = EEPROM.read(pin);
          client.print("PIN ");
          client.print(pin);
          client.print(" read: ");
          client.print(digitalRead(pin));

          ok = true;
        }
        if (size >= 2 && msg[2] == 'Y') { // query values of multiple pins 0-10
          int i=0;
          for (i=0; i<10; i++) {
            client.print(" PIN ");
            client.print(i);
            client.print(": ");
            client.print(digitalRead(i));
          }
          
          ok = true;
        }
      }
      free(msg);
    }

    if (ok == 1) {
      client.print("ACK. WDT: ");
      client.println(wdtvalue);
      
      wdtvalue = WDTINITVALUE;
      LEDindicateActivity();
      
    }

    if (ok == 0) {
      client.println("BAD");
      /*client.println("usage: xxPyyy, e.g. 05P085 - Port 05 PWM 085%");
      client.println("       xxOy,   e.g. 06O1   - Port 06 OUT on");
      client.println("       xxIy,   e.g. 07I    - Read Port 07 Digital Status");
      */  
    }
    client.stop();
  }
}

