#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
char server[] = "lavamundo.com.br";
IPAddress ip(192, 168, 15, 19);
IPAddress myDns(192, 168, 15, 1);
EthernetClient client;
unsigned long beginMicros, endMicros;
unsigned long byteCount = 0;
bool printWebData = true;

const int maquinasPortas[] = { 4, 5, 6, 7, 8, 9 };
const int maquinasIds[] = { 1, 2, 3, 7, 8, 9 };

void initEthernet() {
  Ethernet.init(10);
  Serial.begin(9600);
  while (!Serial) {}

  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
      while (true) {delay(1);}
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    Ethernet.begin(mac, ip, myDns);
  } else {
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
  }
  delay(1000);
  Serial.print("connecting to ");
  Serial.print(server);
  Serial.println("...");

  // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    Serial.print("connected to ");
    Serial.println(client.remoteIP());
  } else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }
  beginMicros = micros();
}

void sendEthernet(int id) {
  
    // Make a HTTP request:
    client.println("POST /iot.php HTTP/1.1");
    client.println("Host: lavamundo.com.br");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println("Connection: close");
    client.println("Content-Length: 20");
    client.println();
    client.print("maquina_id=");
    client.print(id);
    client.println("&emuso=1");
    client.println();
  
}

void initPortas() {
  for (size_t i = 0; i < (sizeof(maquinasPortas) / sizeof(maquinasPortas[0])); i++)
  {
    pinMode(maquinasPortas[i], INPUT_PULLUP);
  }
}

void receiveEthernet() {
  int len = client.available();
  if (len > 0) {
    byte buffer[80];
    if (len > 80) len = 80;
    client.read(buffer, len);
    if (printWebData) {
      Serial.write(buffer, len);
    }
    byteCount = byteCount + len;
  }

  if (!client.connected()) {
    endMicros = micros();
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
    Serial.print("Received ");
    Serial.print(byteCount);
    Serial.print(" bytes in ");
    float seconds = (float)(endMicros - beginMicros) / 1000000.0;
    Serial.print(seconds, 4);
    float rate = (float)byteCount / seconds / 1000.0;
    Serial.print(", rate = ");
    Serial.print(rate);
    Serial.print(" kbytes/second");
    Serial.println();
  }
}

void setup() {
  initEthernet();
  initPortas();
}

void loop() {
  for (size_t i = 0; i < (sizeof(maquinasPortas) / sizeof(maquinasPortas[0])); i++)
  {
    if (!digitalRead(maquinasPortas[i])) {
      sendEthernet(maquinasIds[i]);
      receiveEthernet();
      break;
    }
  }
  
  // debounce
  delay(500);
}
