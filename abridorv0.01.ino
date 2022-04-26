// Import required libraries
#include "EspMQTTClient.h"


const char VERSION[] = "0.01";

// Número de milisegundos de pausa entre chequeo y chequeo del estado de los termostatos. No hay necesidad de estar procesando permanentemente.
const int DELAY_MS = 600; //Duración del pulso de apertura
const int MONITORING_DELAY_MS = 60000; //Mandamos lecturas de monitorizacion cada minuto

String chipId="";

const String mqttBase     = "casa/";
const String mqttType = "/abridor/";
const String mqttMon  = "/monitor/";

String mqttMonThisChip;

unsigned long previousMillisMon = 0; //Control del tiempo de bucle sin delay()

const String tipoNodo = "abridor";
char myname[32];

const char mqttAperturaGaraje[] = "casa/garaje/puerta";
// Pin para activar el rele
const int pinPuerta = D1; // Pin D1

const char *ssid = "<MYSSID>"; // cannot be longer than 32 characters!
const char *pass = "<MYPASS>";
const char *mqtt_server = "<MYMQTTBROKER>";
const int mqtt_port = 1883;
const char *mqttuser = "<MYUSER>";
const char *mqttpass = "<MYMQTTPASS>";


EspMQTTClient client(
  ssid,
  pass,
  mqtt_server,  // MQTT Broker server ip
  "",   // Can be omitted if not needed
  "",   // Can be omitted if not needed
  myname,     // Client name that uniquely identify your device
  mqtt_port              // The MQTT port, default to 1883. this line can be omitted
);

void setup(){
  wifi_country_t country = { .cc = "ES", .schan = 1, .nchan = 13, .policy = WIFI_COUNTRY_POLICY_AUTO };
  String nombreNodo;
  
  // Serial port for debugging purposes
  Serial.begin(115200);
  
  Serial.print(F("Abridor v"));
  Serial.println(VERSION);
  chipId = ESP.getChipId();
  nombreNodo = tipoNodo + chipId;
  Serial.println();
  Serial.print(F("ESP8266 Chip id = "));
  Serial.println(chipId);
  Serial.print(F("Mi nombre: "));
  Serial.println(nombreNodo);
  nombreNodo.toCharArray(myname, nombreNodo.length() + 1);

  String mqttBaseThisChip = mqttBase + chipId;
  String mqttBaseThisChipType = mqttBaseThisChip + mqttType;
  mqttMonThisChip = mqttBaseThisChip + mqttMon;
  
  Serial.print(F("mqttMonThisChip: "));
  Serial.println(mqttMonThisChip);
  wifi_set_country(&country);

  pinMode(pinPuerta, OUTPUT);
  
// Optional functionnalities of EspMQTTClient : 
//  client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
  client.enableHTTPWebUpdater(); // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overrited with enableHTTPWebUpdater("user", "password").
//  client.enableLastWillMessage("TestClient/lastwill", "I am going offline");  // You can activate the retain flag by setting the third parameter to true
}

// This function is called once everything is connected (Wifi and MQTT)
// WARNING : YOU MUST IMPLEMENT IT IF YOU USE EspMQTTClient
void onConnectionEstablished()
{
  wifi_country_t country;
  const String mqttMonThisChipConnectedAtMillis = mqttMonThisChip + "connectedatmillis/";
  const String mqttMonThisChipIP = mqttMonThisChip + "ip/";
  const String mqttMonThisChipHostname = mqttMonThisChip + "hostname/";
  const String mqttMonThisChipVersion = mqttMonThisChip + "version/";
  
  
  // Print WiFi data to serial
  Serial.println();
  Serial.print(F("Connected, IP address: "));
  Serial.println(WiFi.localIP());
  Serial.print(F("Hostname: "));
  Serial.println(WiFi.hostname());
  Serial.print(F("DNS Server: "));
  Serial.print(WiFi.dnsIP());
  Serial.print(F(", "));
  Serial.println(WiFi.dnsIP(1));
  Serial.print(F("RSSI: "));
  Serial.print(WiFi.RSSI());
  Serial.println(F(" dBm"));
  wifi_get_country(&country);
  Serial.print(F("Wifi Country: "));
  Serial.print(country.cc);
  Serial.print(F(" schan: "));
  Serial.print(country.schan);
  Serial.print(F(" nchan: "));
  Serial.print(country.nchan);
  Serial.print(F(" policy: "));
  Serial.println(country.policy);
    
  client.publish(mqttMonThisChipConnectedAtMillis, String(millis()), true);
  client.publish(mqttMonThisChipIP, WiFi.localIP().toString(), true);
  client.publish(mqttMonThisChipHostname, String(WiFi.hostname()), true);
  client.publish(mqttMonThisChipVersion, VERSION, true);

  Serial.println(F("Sending remote data on connection established"));
  reportMonitoringData();

  // Subscribe to mqttAperturaGaraje and display received message to Serial
  client.subscribe(mqttAperturaGaraje, [](const String & topic, const String & payload) {
    Serial.println(topic + ": " + payload);
    if (payload == "on")
    {
      Serial.println(F("Activando relé garaje"));
      aperturaPuerta();
    }
    else
    {
      Serial.println(F("Mensaje no esperado"));
    }
  });
}

void aperturaPuerta()
{
  unsigned long currentMillis;
  unsigned long previousMillis; //Control del tiempo de bucle sin delay()

  digitalWrite(pinPuerta, HIGH);
  previousMillis = millis();
  currentMillis = previousMillis;
  Serial.println(F("Relé activo"));
  while (currentMillis - previousMillis <= DELAY_MS)
  {
    currentMillis = millis();
  }
  digitalWrite(pinPuerta, LOW);
  Serial.println(F("Relé inactivo"));
}

void reportMonitoringData()
{
  const String mqttMonThisChipFreeHeap = mqttMonThisChip + "freeheap/";
  const String mqttMonThisChipUptime = mqttMonThisChip + "uptime/";
  const String mqttMonThisChipRSSI = mqttMonThisChip + "rssi/";
  
  if (client.isConnected())
  {
    client.publish(mqttMonThisChipFreeHeap, String(ESP.getFreeHeap()));
    client.publish(mqttMonThisChipUptime, String(millis()));
    client.publish(mqttMonThisChipRSSI, String(WiFi.RSSI()));
  }
}

void loop(){
  unsigned long currentMillisMon = millis();

  
  client.loop();
  
  if (currentMillisMon - previousMillisMon >= MONITORING_DELAY_MS)
  {
    // save the last time we entered here
    previousMillisMon = currentMillisMon;
    Serial.println(F("Sending remote data every MONITORING_DELAY_MS"));
    reportMonitoringData();
  }
  
}
