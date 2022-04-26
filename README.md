# A Wemos D1 mini relay to open automatic doors
## Relevant parts of this code
Each device gets its unique name from its chip Id

```const String tipoNodo = "abridor";

chipId = ESP.getChipId();
nombreNodo = tipoNodo + chipId;
```
The devide creates an MQTT topic tu publish its information.
First we define some relevant strings that will be used as bricks to create the final topics.
```
const String mqttBase     = "casa/";
const String mqttType = "/abridor/";
const String mqttMon  = "/monitor/";
```
Then we create some strings to store the final topics. Since this device will only be listening for requests it will only publish its monitoring information. We could improve this by publishing that the device has actually activated the relay.
```
String mqttMonThisChip;
```
Then we build the final topics that will be specific for this device.
``` 
  String mqttBaseThisChip = mqttBase + chipId;
  String mqttBaseThisChipType = mqttBaseThisChip + mqttType;
  mqttMonThisChip = mqttBaseThisChip + mqttMon;
```
The device subscribes to the MQTT topic where requests for door opening will be published.
```
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
```
And set to cycle the voltage level of the pin that will activate the relay for DELAY_MS (600ms in my case). This will activate the door motor.
```
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
