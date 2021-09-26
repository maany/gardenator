#if defined(ESP32)
#include <WiFiMulti.h>
WiFiMulti wifiMulti;
#define DEVICE "Gardenator-Chives"
#elif defined(ESP8266)
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;
#define DEVICE "ESP8266"
#endif

//OTA
#include <ArduinoOTA.h>

// InfluxDB
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

// WiFi AP SSID
#define WIFI_SSID "SFR-ee08"
// WiFi password
#define WIFI_PASSWORD "doesntmatter"
// InfluxDB v2 server url, e.g. https://eu-central-1-1.aws.cloud2.influxdata.com (Use: InfluxDB UI -> Load Data -> Client Libraries)
#define INFLUXDB_URL "http://maany.synology.me:38086"
// InfluxDB v2 server or cloud API authentication token (Use: InfluxDB UI -> Data -> Tokens -> <select token>)
#define INFLUXDB_TOKEN "3xCoXKBKtG8444qIvKVC7jbWro7A40Ruut9jaKtFsx4IMJgKPPfKyerJ4t3cbZ64Pbvx8SfRq-oJmlGP83nEIA=="
// InfluxDB v2 organization id (Use: InfluxDB UI -> User -> About -> Common Ids )
#define INFLUXDB_ORG "prada"
// InfluxDB v2 bucket name (Use: InfluxDB UI ->  Data -> Buckets)
#define INFLUXDB_BUCKET "gardenator"


// Set timezone string according to https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
// Examples:
//  Pacific Time: "PST8PDT"
//  Eastern: "EST5EDT"
//  Japanesse: "JST-9"
//  Central Europe: "CET-1CEST,M3.5.0,M10.5.0/3"
#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3"

// InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);


// Relays
#define PUMP_RELAY 26

// Sensors
#define SOIL_MOISTURE_SENSOR 34
#define LIGHT_SENSOR 35

// Thresholds
#define PUMP_SENSOR_THRESHOLD 150

// Variables for sensor values
int moisture_value = 0;
int light_value = 0;


// Data points for Sensors
Point soil_moisture_data_point("soil_moisture");
Point light_data_point("light");
 
void setup() {
  Serial.begin(115200);

  // Setup wifi
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to wifi");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();
  
  // OTA Setup
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();
  Serial.println("Over the Air Programming Configured!");
  // Add tags
  soil_moisture_data_point.addTag("device", DEVICE);
  soil_moisture_data_point.addTag("SSID", WiFi.SSID());
  light_data_point.addTag("device", DEVICE);
  light_data_point.addTag("SSID", WiFi.SSID());

  // Accurate time is necessary for certificate validation and writing in batches
  // For the fastest time sync find NTP servers in your area: https://www.pool.ntp.org/zone/
  // Syncing progress and the time will be printed to Serial.
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  // Pin modes
  pinMode(PUMP_RELAY, OUTPUT);
}

void loop() {
  // Check OTA updates
  ArduinoOTA.handle();
  
  // Clear fields for reusing the point. Tags will remain untouched
  soil_moisture_data_point.clearFields();
  light_data_point.clearFields();

  // Store measured value into point
  // Report RSSI of currently connected network
  soil_moisture_data_point.addField("rssi", WiFi.RSSI());
  light_data_point.addField("rssi", WiFi.RSSI());

  // Get sensor values
  moisture_value = analogRead(SOIL_MOISTURE_SENSOR);
  light_value = analogRead(LIGHT_SENSOR);

  // Add sensor-value fields for InfluxDB
  soil_moisture_data_point.addField("value", moisture_value);
  light_data_point.addField("value", light_value);
  
  // Print what are we exactly writing
  Serial.print("Writing: ");
  Serial.println(soil_moisture_data_point.toLineProtocol());
  Serial.println(light_data_point.toLineProtocol());

  // If no Wifi signal, try to reconnect it
  if ((WiFi.RSSI() == 0) && (wifiMulti.run() != WL_CONNECTED)) {
    Serial.println("Wifi connection lost");
  }

  // Write moisture value
  if (!client.writePoint(soil_moisture_data_point)) {
    Serial.print("InfluxDB write for soil moisture failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  // Write light value
  if (!client.writePoint(light_data_point)) {
    Serial.print("InfluxDB write for light sensor failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  //Wait 10s
  Serial.println("Wait 10s");
  delay(10000);
 

  // Pump control
  while (moisture_value < PUMP_SENSOR_THRESHOLD) {
      Serial.println("Pump is currently ON!! ");
      Serial.println(moisture_value);
      digitalWrite(PUMP_RELAY, HIGH);
      moisture_value = analogRead(SOIL_MOISTURE_SENSOR);
      delay(500);
  }
 
  Serial.println("PUMP is currently OFF!! ");
  digitalWrite(PUMP_RELAY, LOW);
  
  
  /*
  digitalWrite(PUMP_RELAY, HIGH);
  Serial.println("Pump is currently ON!! ");
  delay(10000);
  digitalWrite(PUMP_RELAY, LOW);
  Serial.println("PUMP is currently OFF!! ");
  delay(10000);
  */
  
}