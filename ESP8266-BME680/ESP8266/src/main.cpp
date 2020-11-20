#include <Arduino.h>
#include <main.h>

#include <bsec.h>
#include <ESP8266WiFiMulti.h>
#include <InfluxDbClient.h>

#define LED_BUILTIN 2

ESP8266WiFiMulti wifiMulti;
Bsec iaqSensor;

// InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_DB_NAME);

Point sensor("air-log");

const uint32_t connectTimeoutMs = 5000;
const uint32_t wifi_check_loop = 5000;
const uint32_t sensor_check_loop = 5000;
uint32_t wifi_check_timer = 0;
uint32_t sensor_check_timer = 0;

String output;

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  Wire.begin();

  Serial.println();
  Serial.println();
  Serial.println();

  WiFi.mode(WIFI_STA);
  wifiMulti.addAP("Deco_M4", "networkpass");

  iaqSensor.begin(BME680_I2C_ADDR_PRIMARY, Wire);
  output = "\nBSEC library version " + String(iaqSensor.version.major) + "." + String(iaqSensor.version.minor) + "." + String(iaqSensor.version.major_bugfix) + "." + String(iaqSensor.version.minor_bugfix);
  Serial.println(output);
  checkIaqSensorStatus();

  bsec_virtual_sensor_t sensorList[10] = {
    BSEC_OUTPUT_RAW_TEMPERATURE,
    BSEC_OUTPUT_RAW_PRESSURE,
    BSEC_OUTPUT_RAW_HUMIDITY,
    BSEC_OUTPUT_RAW_GAS,
    BSEC_OUTPUT_IAQ,
    BSEC_OUTPUT_STATIC_IAQ,
    BSEC_OUTPUT_CO2_EQUIVALENT,
    BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
  };

  iaqSensor.setTemperatureOffset(3);
  iaqSensor.updateSubscription(sensorList, 10, BSEC_SAMPLE_RATE_LP);
  checkIaqSensorStatus();

  sensor.addTag("device", "esp8266");
  
  // "Timestamp [ms], raw temperature [°C], pressure [hPa], raw relative humidity [%], gas [Ohm], IAQ, IAQ accuracy, temperature [°C], relative humidity [%], Static IAQ, CO2 equivalent, breath VOC equivalent";

  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
}


void loop()
{
  if(millis()-wifi_check_timer > wifi_check_loop)
  {
    wifi_check_timer = millis();

    if (wifiMulti.run(connectTimeoutMs) == WL_CONNECTED) {
      digitalWrite(LED_BUILTIN, LOW);
      // Serial.print("WiFi connected: ");
      // Serial.print(WiFi.SSID());
      // Serial.print(" ");
      // Serial.println(WiFi.localIP());
    } else {
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.println("WiFi not connected!");
    }
  }

  if(millis()-sensor_check_timer > sensor_check_loop)
  if(1)
  {
    sensor_check_timer = millis();

    if (iaqSensor.run()) {
      Serial.println("{");
      Serial.print("Temp: ");  Serial.println(iaqSensor.temperature);
      Serial.print("RawTemp: ");  Serial.println(iaqSensor.rawTemperature);
      Serial.print("Humidity: ");  Serial.println(iaqSensor.humidity);
      Serial.print("Pressure: ");  Serial.println(iaqSensor.pressure);
      Serial.print("IAQ: ");  Serial.println(iaqSensor.iaq);
      Serial.print("staticIAQ: ");  Serial.println(iaqSensor.staticIaq);
      Serial.print("staticIAQ Accuracy: ");  Serial.println(iaqSensor.staticIaqAccuracy);
      Serial.print("IAQ: ");  Serial.println(iaqSensor.iaq);
      Serial.print("IAQ Accuracy: ");  Serial.println(iaqSensor.iaqAccuracy);
      Serial.print("CO2 Equivalent: ");  Serial.println(iaqSensor.co2Equivalent);
      Serial.print("CO2 Accuracy: ");  Serial.println(iaqSensor.co2Accuracy);
      Serial.print("Breath Equivalent: ");  Serial.println(iaqSensor.breathVocEquivalent);
      Serial.println("}");
      Serial.println();

      char buf[100] = { 0 };

      sensor.clearFields();
      
      sprintf(buf, "%f", iaqSensor.temperature);
      sensor.addField("Temperature", buf);
      memset(buf, 0, 100);
      
      sprintf(buf, "%f", iaqSensor.humidity);
      sensor.addField("Humidity", buf);
      memset(buf, 0, 100);
      
      sprintf(buf, "%f", iaqSensor.pressure);
      sensor.addField("Pressure", buf);
      memset(buf, 0, 100);
      
      sprintf(buf, "%f", iaqSensor.compGasValue);
      sensor.addField("CompGas", buf);
      memset(buf, 0, 100);
      
      sprintf(buf, "%f", iaqSensor.gasPercentage);
      sensor.addField("Gas Percentage", buf);
      memset(buf, 0, 100);
      
      sprintf(buf, "%f", iaqSensor.iaq);
      sensor.addField("IAQ", buf);
      memset(buf, 0, 100);
      
      sprintf(buf, "%f", iaqSensor.staticIaq);
      sensor.addField("static IAQ", buf);
      memset(buf, 0, 100);
      
      sprintf(buf, "%f", iaqSensor.co2Equivalent);
      sensor.addField("CO2 Equivalent", buf);
      memset(buf, 0, 100);
      
      sprintf(buf, "%f", iaqSensor.breathVocEquivalent);
      sensor.addField("Breath VOC Equivalent", buf);
      memset(buf, 0, 100);
      
      sprintf(buf, "%f", iaqSensor.co2Accuracy);
      sensor.addField("CO2 Accuracy", buf);
      memset(buf, 0, 100);
      
      sprintf(buf, "%f", iaqSensor.iaqAccuracy);
      sensor.addField("IAQ Accuracy", buf);
      memset(buf, 0, 100);
      
      sprintf(buf, "%f", iaqSensor.compGasAccuracy);
      sensor.addField("CompGas Accuracy", buf);
      memset(buf, 0, 100);
      
      Serial.println();
      if(client.writePoint(sensor))
      {
        Serial.println("InfluxDB write success");
      }else
      {
        Serial.println("InfluxDB failed to write!");
      }
      Serial.println();

    } else {
      checkIaqSensorStatus();

      digitalWrite(LED_BUILTIN, HIGH);
    }
  }
}

void checkIaqSensorStatus()
{
  if (iaqSensor.status != BSEC_OK) {
    if (iaqSensor.status < BSEC_OK) {
      output = "BSEC error code : " + String(iaqSensor.status);
      Serial.println(output);
      for (;;)
        errLeds(); /* Halt in case of failure */
    } else {
      output = "BSEC warning code : " + String(iaqSensor.status);
      Serial.println(output);
    }
  }

  if (iaqSensor.bme680Status != BME680_OK) {
    if (iaqSensor.bme680Status < BME680_OK) {
      output = "BME680 error code : " + String(iaqSensor.bme680Status);
      Serial.println(output);
      for (;;)
        errLeds(); /* Halt in case of failure */
    } else {
      output = "BME680 warning code : " + String(iaqSensor.bme680Status);
      Serial.println(output);
    }
  }
}

void errLeds()
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
}
