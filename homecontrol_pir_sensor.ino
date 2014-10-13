#include <Ethernet.h>
#include <SPI.h>
#include <MemoryFree.h>

#define BUTTON 8             // Push button is Input#8
#define LED 3                // LED indicator
#define PIR 4                // PIR sensor output

boolean activate = false;    // If the user has pressed the button, then activate == True.  Starts out == False.
int result = 0;              // How many times has the motion sensor tripped?
int lastTime = 0;            // timestamp letzter Durchlauf
int duration = 4000;         // Dauer eines Durchlaufs
int signalDuration = 1000;   // Dauer des LED Signals 

long sensorId = 37643;

byte pi_adress[] =  {192, 168, 1, 98};

unsigned char _mac[]  = {0xDF, 0x8D, 0xCB, 0x37, 0xC4, 0xED  };
unsigned char _ip[]   = { 192, 168, 1, 20 };
unsigned char _dns[]  = { 192, 168, 1, 1  };
unsigned char _gate[] = { 192, 168, 1, 1  };
unsigned char _mask[] = { 255, 255, 255, 0  };

// Variablen für Netzwerkdienste
EthernetClient interfaceClient;

const int  MAX_BUFFER_LEN           = 80; // max characters in page name/parameter 
char       buffer[MAX_BUFFER_LEN+1]; // additional character for terminating null


/**
 *
 */
void setup()
{
  Serial.begin(9600);          // Ready the unit for 9600 baud serial communication with the Arduino (Serial Monitor).
  delay(1000);

  pinMode(LED, OUTPUT);         // Indicator LED = output
  pinMode(PIR, INPUT);         // Motion Sensor = input
  pinMode(BUTTON, INPUT);     // Set Button as digital Input

  while(!Serial){
  }

  digitalWrite(LED, HIGH);

  // Netzwerk initialisieren
  Ethernet.begin(_mac, _ip, _dns, _gate, _mask);
  
  Serial.println("HomeControl - ICU Sensor v1");
  Serial.println();

  delay(1000);
  digitalWrite(LED, LOW);
}

/**
 *
 */
void loop(){
  int time = millis();
  while ((lastTime+duration) > time ) {
    if (digitalRead(BUTTON) == HIGH)  {
       Serial.println("Button pressed");
       
       if(activate == true){
         digitalWrite(LED, LOW);
         activate = false;
       } else {
         digitalWrite(LED, HIGH);
         activate = true;         
       }
       delay(1000);
    }
    
    if(activate == true){
      if (digitalRead(PIR) == HIGH) {
        Serial.print(time);
        Serial.println(": Motion Detected ");
// ----------------              
        if (interfaceClient.connect(pi_adress, 80)) {
          delay(800);
          Serial.print("Rufe URL auf: GET /signalInput.php?sensorId=");
          Serial.println(sensorId);
//          interfaceClient.print("GET /homecontrol.sql");
          interfaceClient.print("GET /signalInput.php?sensorId=");
          interfaceClient.print(sensorId);
          interfaceClient.println(" HTTP/1.1");
          interfaceClient.println("Host: 192.168.1.98");
          interfaceClient.println("Connection: close");
          interfaceClient.println();

          unsigned long startTime = millis();
          while (!interfaceClient.available() && (millis() - startTime ) < 5000) {
          }

          while (interfaceClient.available()) {
            char c = interfaceClient.read();
            Serial.print(c);
          }
          
          if (interfaceClient.connected()) {
            Serial.println("disconnecting.");
            interfaceClient.stop();
          }
// ----------------              

        } else {
          Serial.println("PI - Connection Error!!!");
        }
        delay(1000);

        digitalWrite(LED, LOW);
        delay(signalDuration);
        digitalWrite(LED, HIGH);
      }
    }
  }
  lastTime = time;
}


void sendMotionDetectSignal(){
  if (interfaceClient.connect(pi_adress, 80)) {
    delay(100);
    interfaceClient.print(F("GET /signalInput.php?sensorId="));
    interfaceClient.print(sensorId);
    interfaceClient.println(F(" HTTP/1.1"));
//    interfaceClient.print(F("Host: "));
//    interfaceClient.println(pi_adress);
    interfaceClient.println(F("Connection: close"));
    interfaceClient.println();

    readResponse(interfaceClient);

  } else {
    Serial.println("PI - Connection Error!!!");
  }

  interfaceClient.stop();  
}


void readResponse(EthernetClient client){
  unsigned long startTime = millis();

  while ((!client.available()) && ((millis() - startTime ) < 5000)){ 
    // waiting for server   
  };

  char* p_ret = readFromClientInterface(client);
  Serial.println(p_ret);
  p_ret = NULL;
}



char* readFromClientInterface(EthernetClient client){
  memset(buffer,0, sizeof(buffer)); // clear the buffer
  boolean reading = false;

  int i = 0;

  while (client.available()) {
    char c = client.read();     

      *(buffer+i) = c;
      i++;  
  }
  *(buffer+i)='\0';  

  return buffer;
}

