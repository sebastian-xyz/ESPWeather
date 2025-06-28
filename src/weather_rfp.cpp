#include "math.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <memory>
#include <time.h>
#include "weather_rfp.hpp"

const char *root_cert =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIF3jCCA8agAwIBAgIQAf1tMPyjylGoG7xkDjUDLTANBgkqhkiG9w0BAQwFADCB\n"
    "iDELMAkGA1UEBhMCVVMxEzARBgNVBAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0pl\n"
    "cnNleSBDaXR5MR4wHAYDVQQKExVUaGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNV\n"
    "BAMTJVVTRVJUcnVzdCBSU0EgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMTAw\n"
    "MjAxMDAwMDAwWhcNMzgwMTE4MjM1OTU5WjCBiDELMAkGA1UEBhMCVVMxEzARBgNV\n"
    "BAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0plcnNleSBDaXR5MR4wHAYDVQQKExVU\n"
    "aGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNVBAMTJVVTRVJUcnVzdCBSU0EgQ2Vy\n"
    "dGlmaWNhdGlvbiBBdXRob3JpdHkwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIK\n"
    "AoICAQCAEmUXNg7D2wiz0KxXDXbtzSfTTK1Qg2HiqiBNCS1kCdzOiZ/MPans9s/B\n"
    "3PHTsdZ7NygRK0faOca8Ohm0X6a9fZ2jY0K2dvKpOyuR+OJv0OwWIJAJPuLodMkY\n"
    "tJHUYmTbf6MG8YgYapAiPLz+E/CHFHv25B+O1ORRxhFnRghRy4YUVD+8M/5+bJz/\n"
    "Fp0YvVGONaanZshyZ9shZrHUm3gDwFA66Mzw3LyeTP6vBZY1H1dat//O+T23LLb2\n"
    "VN3I5xI6Ta5MirdcmrS3ID3KfyI0rn47aGYBROcBTkZTmzNg95S+UzeQc0PzMsNT\n"
    "79uq/nROacdrjGCT3sTHDN/hMq7MkztReJVni+49Vv4M0GkPGw/zJSZrM233bkf6\n"
    "c0Plfg6lZrEpfDKEY1WJxA3Bk1QwGROs0303p+tdOmw1XNtB1xLaqUkL39iAigmT\n"
    "Yo61Zs8liM2EuLE/pDkP2QKe6xJMlXzzawWpXhaDzLhn4ugTncxbgtNMs+1b/97l\n"
    "c6wjOy0AvzVVdAlJ2ElYGn+SNuZRkg7zJn0cTRe8yexDJtC/QV9AqURE9JnnV4ee\n"
    "UB9XVKg+/XRjL7FQZQnmWEIuQxpMtPAlR1n6BB6T1CZGSlCBst6+eLf8ZxXhyVeE\n"
    "Hg9j1uliutZfVS7qXMYoCAQlObgOK6nyTJccBz8NUvXt7y+CDwIDAQABo0IwQDAd\n"
    "BgNVHQ4EFgQUU3m/WqorSs9UgOHYm8Cd8rIDZsswDgYDVR0PAQH/BAQDAgEGMA8G\n"
    "A1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEMBQADggIBAFzUfA3P9wF9QZllDHPF\n"
    "Up/L+M+ZBn8b2kMVn54CVVeWFPFSPCeHlCjtHzoBN6J2/FNQwISbxmtOuowhT6KO\n"
    "VWKR82kV2LyI48SqC/3vqOlLVSoGIG1VeCkZ7l8wXEskEVX/JJpuXior7gtNn3/3\n"
    "ATiUFJVDBwn7YKnuHKsSjKCaXqeYalltiz8I+8jRRa8YFWSQEg9zKC7F4iRO/Fjs\n"
    "8PRF/iKz6y+O0tlFYQXBl2+odnKPi4w2r78NBc5xjeambx9spnFixdjQg3IM8WcR\n"
    "iQycE0xyNN+81XHfqnHd4blsjDwSXWXavVcStkNr/+XeTWYRUc+ZruwXtuhxkYze\n"
    "Sf7dNXGiFSeUHM9h4ya7b6NnJSFd5t0dCy5oGzuCr+yDZ4XUmFF0sbmZgIn/f3gZ\n"
    "XHlKYC6SQK5MNyosycdiyA5d9zZbyuAlJQG03RoHnHcAP9Dc1ew91Pq7P8yF1m9/\n"
    "qS3fuQL39ZeatTXaw2ewh0qpKJ4jjv9cJ2vhsE/zB+4ALtRZh8tSQZXq9EfX7mRB\n"
    "VXyNWQKV3WKdwrnuWih0hKWbt5DHDAff9Yk2dDLWKMGwsAvgnEzDHNb842m1R0aB\n"
    "L6KCq9NjRHDEjf8tM7qtj3u1cIiuPhnPQCjY/MiQu12ZIvVS5ljFH4gxQ+6IHdfG\n"
    "jjxDah2nGN59PRbxYvnKkKj9\n"
    "-----END CERTIFICATE-----\n";

WeatherRFP::WeatherRFP(float latitude, float longitude)
{
  this->num_hours = ESPWeatherNumHours;
  this->user_agent = ESPWeatherUserAgent;
  this->longitude = longitude;
  this->latitude = latitude;
  this->altitude = 0;
  this->local_time = new tm;
  this->expired_time = new tm;
  this->temperature = new WeatherDataRFP(this->num_hours, ESPWeatherRFPFactorTemperature);
  this->dew_point = new WeatherDataRFP(this->num_hours, ESPWeatherRFPFactorDewPoint);
  this->precipitation = new WeatherDataRFP(this->num_hours, ESPWeatherRFPFactorPrecipitation);
  this->wind_speeds = new WeatherDataRFP(this->num_hours, ESPWeatherRFPFactorWindSpeed);
  this->wind_direction = new WeatherDataRFP(this->num_hours, ESPWeatherRFPFactorWindDirection);
  this->air_pressure = new WeatherDataRFP(this->num_hours, ESPWeatherRFPFactorAirPressure);
  this->cloudiness = new WeatherDataRFP(this->num_hours, ESPWeatherRFPFactorCloudiness);
  this->relative_humidity = new WeatherDataRFP(this->num_hours, ESPWeatherRFPFactorRelativeHumidity);
  this->last_modified = "";
  this->symbol_code_next_1h = "";
  this->symbol_code_next_12h = "";
  this->symbol_code_next_6h = "";
  this->utc_offset = 0;
  this->daylight_saving = false;
  this->expired_time->tm_year = 0; // Years since 1900
  this->expired_time->tm_mon = 0;  // Months since January (0-11)
  this->expired_time->tm_mday = 0; // Day of the month
  this->expired_time->tm_hour = 0;
  this->expired_time->tm_min = 0;
  this->expired_time->tm_sec = 0;
}
WeatherRFP::WeatherRFP(uint8_t num_hours, float latitude, float longitude)
{
  this->num_hours = num_hours;
  this->user_agent = ESPWeatherUserAgent;
  this->longitude = longitude;
  this->latitude = latitude;
  this->altitude = 0;
  this->local_time = new tm;
  this->expired_time = new tm;
  this->temperature = new WeatherDataRFP(this->num_hours, ESPWeatherRFPFactorTemperature);
  this->dew_point = new WeatherDataRFP(this->num_hours, ESPWeatherRFPFactorDewPoint);
  this->precipitation = new WeatherDataRFP(this->num_hours, ESPWeatherRFPFactorPrecipitation);
  this->wind_speeds = new WeatherDataRFP(this->num_hours, ESPWeatherRFPFactorWindSpeed);
  this->wind_direction = new WeatherDataRFP(this->num_hours, ESPWeatherRFPFactorWindDirection);
  this->air_pressure = new WeatherDataRFP(this->num_hours, ESPWeatherRFPFactorAirPressure);
  this->cloudiness = new WeatherDataRFP(this->num_hours, ESPWeatherRFPFactorCloudiness);
  this->relative_humidity = new WeatherDataRFP(this->num_hours, ESPWeatherRFPFactorRelativeHumidity);
  this->last_modified = "";
  this->symbol_code_next_1h = "";
  this->symbol_code_next_12h = "";
  this->symbol_code_next_6h = "";
  this->utc_offset = 0;
  this->daylight_saving = false;
  this->expired_time->tm_year = 0; // Years since 1900
  this->expired_time->tm_mon = 0;  // Months since January (0-11)
  this->expired_time->tm_mday = 0; // Day of the month
  this->expired_time->tm_hour = 0;
  this->expired_time->tm_min = 0;
  this->expired_time->tm_sec = 0;
}
WeatherRFP::WeatherRFP(float latitude, float longitude, uint16_t altitude)
{
  this->num_hours = ESPWeatherNumHours;
  this->user_agent = ESPWeatherUserAgent;
  this->longitude = longitude;
  this->latitude = latitude;
  this->altitude = altitude;
  this->local_time = new tm;
  this->expired_time = new tm;
  this->temperature = new WeatherDataRFP(this->num_hours, ESPWeatherRFPFactorTemperature);
  this->dew_point = new WeatherDataRFP(this->num_hours, ESPWeatherRFPFactorDewPoint);
  this->precipitation = new WeatherDataRFP(this->num_hours, ESPWeatherRFPFactorPrecipitation);
  this->wind_speeds = new WeatherDataRFP(this->num_hours, ESPWeatherRFPFactorWindSpeed);
  this->wind_direction = new WeatherDataRFP(this->num_hours, ESPWeatherRFPFactorWindDirection);
  this->air_pressure = new WeatherDataRFP(this->num_hours, ESPWeatherRFPFactorAirPressure);
  this->cloudiness = new WeatherDataRFP(this->num_hours, ESPWeatherRFPFactorCloudiness);
  this->relative_humidity = new WeatherDataRFP(this->num_hours, ESPWeatherRFPFactorRelativeHumidity);
  this->last_modified = "";
  this->symbol_code_next_1h = "";
  this->symbol_code_next_12h = "";
  this->symbol_code_next_6h = "";
  this->daylight_saving = false;
  this->utc_offset = 0;
  this->expired_time->tm_year = 0; // Years since 1900
  this->expired_time->tm_mon = 0;  // Months since January (0-11)
  this->expired_time->tm_mday = 0; // Day of the month
  this->expired_time->tm_hour = 0;
  this->expired_time->tm_min = 0;
  this->expired_time->tm_sec = 0;
}
WeatherRFP::WeatherRFP(uint8_t num_hours, float latitude, float longitude, uint16_t altitude)
{
  this->num_hours = num_hours;
  this->user_agent = ESPWeatherUserAgent;
  this->longitude = longitude;
  this->latitude = latitude;
  this->altitude = altitude;
  this->local_time = new tm;
  this->expired_time = new tm;
  this->temperature = new WeatherDataRFP(this->num_hours, ESPWeatherRFPFactorTemperature);
  this->dew_point = new WeatherDataRFP(this->num_hours, ESPWeatherRFPFactorDewPoint);
  this->precipitation = new WeatherDataRFP(this->num_hours, ESPWeatherRFPFactorPrecipitation);
  this->wind_speeds = new WeatherDataRFP(this->num_hours, ESPWeatherRFPFactorWindSpeed);
  this->wind_direction = new WeatherDataRFP(this->num_hours, ESPWeatherRFPFactorWindDirection);
  this->air_pressure = new WeatherDataRFP(this->num_hours, ESPWeatherRFPFactorAirPressure);
  this->cloudiness = new WeatherDataRFP(this->num_hours, ESPWeatherRFPFactorCloudiness);
  this->relative_humidity = new WeatherDataRFP(this->num_hours, ESPWeatherRFPFactorRelativeHumidity);
  this->last_modified = "";
  this->symbol_code_next_1h = "";
  this->symbol_code_next_12h = "";
  this->symbol_code_next_6h = "";
  this->daylight_saving = false;
  this->utc_offset = 0;
  this->expired_time->tm_year = 0; // Years since 1900
  this->expired_time->tm_mon = 0;  // Months since January (0-11)
  this->expired_time->tm_mday = 0; // Day of the month
  this->expired_time->tm_hour = 0;
  this->expired_time->tm_min = 0;
  this->expired_time->tm_sec = 0;
}

void WeatherRFP::set_daylight_saving(bool daylight_saving)
{
  this->daylight_saving = daylight_saving;
}

void WeatherRFP::set_utc_offset(int8_t utc_offset)
{
  this->utc_offset = utc_offset;
}

WeatherRFP::~WeatherRFP()
{
  delete this->local_time;
  delete this->expired_time;
  delete this->temperature;
  delete this->dew_point;
  delete this->precipitation;
  delete this->wind_speeds;
  delete this->wind_direction;
  delete this->air_pressure;
  delete this->cloudiness;
  delete this->relative_humidity;
}

void WeatherRFP::update_location(float latitude, float longitude)
{
  this->longitude = longitude;
  this->latitude = latitude;
}
void WeatherRFP::update_location(float latitude, float longitude,
                                 uint16_t altitude)
{
  this->longitude = longitude;
  this->latitude = latitude;
  this->altitude = altitude;
}

bool WeatherRFP::update_data(fs::FS &fs)
{
  std::unique_ptr<WiFiClientSecure> client(new WiFiClientSecure);
  std::unique_ptr<char[]> buffer(new char[512]);
  client->setCACert(root_cert);
  HTTPClient https;
  sprintf(buffer.get(), "?lat=%.2f&lon=%.2f&altitude=%d", this->latitude,
          this->longitude, this->altitude);
  String url = (String)this->url + (String)buffer.get();
  https.begin(*client, url.c_str());
  https.setUserAgent(this->user_agent);
  if (!this->last_modified.isEmpty())
  {
    https.addHeader("If-Modifed-Since", this->last_modified);
  }
  const char *headerKeys[] = {"last-modified", "expires"};
  const size_t headerKeysCount = sizeof(headerKeys) / sizeof(headerKeys[0]);
  https.collectHeaders(headerKeys, headerKeysCount);

  JsonDocument filter;

  for (uint8_t i = 0; i < this->num_hours + 1; ++i)
  {
    filter["properties"]["timeseries"][i] = true;
    filter["properties"]["timeseries"][i]["data"]["instant"]["details"]
          ["air_temperature"] = true;
    filter["properties"]["timeseries"][i]["data"]["next_1_hours"]["details"]
          ["precipitation_amount"] = true;
    filter["properties"]["timeseries"][i]["data"]["instant"]["details"]
          ["wind_speed"] = true;
    filter["properties"]["timeseries"][i]["data"]["instant"]["details"]
          ["wind_from_direction"] = true;
    filter["properties"]["timeseries"][i]["data"]["instant"]["details"]
          ["air_pressure_at_sea_level"] = true;
    filter["properties"]["timeseries"][i]["data"]["instant"]["details"]
          ["cloud_area_fraction"] = true;
    filter["properties"]["timeseries"][i]["data"]["instant"]["details"]
          ["relative_humidity"] = true;
    filter["properties"]["timeseries"][i]["data"]["instant"]["details"]
          ["dew_point_temperature"] = true;
    if (i == 0)
    {
      filter["properties"]["timeseries"][i]["data"]["next_1_hours"]["summary"]
            ["symbol_code"] = true;
    }
  }

  int httpResponseCode = https.GET();
  String payload = "{}";
  if (httpResponseCode > 0)
  {
#ifdef DEBUG_WEATHER
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
#endif
    if ((!this->last_modified.isEmpty()) && httpResponseCode == 304)
    {
      int header_collected = https.headers();
#ifdef DEBUG_WEATHER
      Serial.println("Data unchanged. Nothing todo");
      Serial.print("Collected ");
      Serial.print(header_collected);
      Serial.println(" headers:");
#endif
      if (header_collected == 2)
      {
        this->last_modified = https.header("last-modified");
        String expires = https.header("expires");
        const char *expires_c = expires.c_str();
        char *end = strptime(expires_c, "%a, %d %b %Y %H:%M:%S GMT",
                             this->expired_time);
#ifdef DEBUG_WEATHER
        if ((end == NULL) || end != "\0")
        {
          Serial.print("Found remaining char: ");
          Serial.println(end);
        }
#endif
#ifdef DEBUG_WEATHER

        Serial.print("last-modified: ");
        Serial.println(this->last_modified);

        Serial.print("expires: ");
        Serial.println(expires);
#endif
      }
      return;
    }
  }
  else
  {
#ifdef DEBUG_WEATHER
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
#endif
  }

  JsonDocument doc;
#endif
  File file = fs.open(scratch_file, FILE_WRITE);
  https.writeToStream(&file);
  file.close();
  file = fs.open(scratch_file);
  if(!file || file.isDirectory()){
#if DEBUG_WEATHER
      Serial.println("- failed to open file for reading");
#endif
      return false;
  }

#ifdef DEBUG_WEATHER
  Serial.println("Starting deserialization");
#endif
  DeserializationError error = deserializeJson(doc, file, DeserializationOption::Filter(filter));
  // while(file.available()){
  //     Serial.write(file.read());
  // }
  file.close();
#if DEBUG_WEATHER
  Serial.println("- done!");
#endif
  // ReadLoggingStream loggingStream(https.getStream(), Serial);
  // DeserializationError error = deserializeJson(doc, loggingStream, DeserializationOption::Filter(filter));

  if (error)
  {

#ifdef DEBUG_WEATHER
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
#endif
    return;
  }
  if (!(doc.containsKey("properties")))
  {
    return;
  }
  if (!(doc["properties"].containsKey("timeseries")))
  {
    return;
  }
  JsonArray timeseries = doc["properties"]["timeseries"];
  float *temps = new float[this->num_hours + 1];
  float *precipitation = new float[this->num_hours + 1];
  float *wind_speeds = new float[this->num_hours + 1];
  float *wind_directions = new float[this->num_hours + 1];
  float *air_pressure = new float[this->num_hours + 1];
  float *cloudiness = new float[this->num_hours + 1];
  float *relative_humidity = new float[this->num_hours + 1];
  float *dew_point = new float[this->num_hours + 1];
  JsonObject current_timeseries_data;
  JsonObject current_timeseries_details;
  JsonObject current_timeseries_next_hour_details;
  if (timeseries[0]["data"].containsKey("next_1_hours"))
  {
    if (timeseries[0]["data"]["next_1_hours"].containsKey("summary"))
    {
      if (timeseries[0]["data"]["next_1_hours"]["summary"].containsKey(
              "symbol_code"))
      {
        this->symbol_code_next_1h =
            String((const char *)timeseries[0]["data"]["next_1_hours"]
                                           ["summary"]["symbol_code"]);
      }
    }
  }
  if (timeseries[0]["data"].containsKey("next_6_hours"))
  {
    if (timeseries[0]["data"]["next_6_hours"].containsKey("summary"))
    {
      if (timeseries[0]["data"]["next_6_hours"]["summary"].containsKey(
              "symbol_code"))
      {
        this->symbol_code_next_6h =
            String((const char *)timeseries[0]["data"]["next_6_hours"]
                                           ["summary"]["symbol_code"]);
      }
    }
  }
  if (timeseries[0]["data"].containsKey("next_12_hours"))
  {
    if (timeseries[0]["data"]["next_12_hours"].containsKey("summary"))
    {
      if (timeseries[0]["data"]["next_12_hours"]["summary"].containsKey(
              "symbol_code"))
      {
        this->symbol_code_next_12h =
            String((const char *)timeseries[0]["data"]["next_12_hours"]
                                           ["summary"]["symbol_code"]);
      }
    }
  }
  for (uint8_t i = 0; i < this->num_hours + 1; ++i)
  {
    current_timeseries_data = timeseries[i]["data"];
    current_timeseries_details = current_timeseries_data["instant"]["details"];
    current_timeseries_next_hour_details =
        current_timeseries_data["next_1_hours"]["details"];

    temps[i] = current_timeseries_details["air_temperature"];
    precipitation[i] =
        current_timeseries_next_hour_details["precipitation_amount"];
    wind_speeds[i] = current_timeseries_details["wind_speed"];
    wind_directions[i] = current_timeseries_details["wind_from_direction"];
    air_pressure[i] = current_timeseries_details["air_pressure_at_sea_level"];
    cloudiness[i] = current_timeseries_details["cloud_area_fraction"];
    relative_humidity[i] = current_timeseries_details["relative_humidity"];
    dew_point[i] = current_timeseries_details["dew_point_temperature"];
  }
  // Free resources
  this->temperature->update_vals(temps);
  this->precipitation->update_vals(precipitation);
  this->wind_speeds->update_vals(wind_speeds);
  this->wind_direction->update_vals(wind_directions);
  this->air_pressure->update_vals(air_pressure);
  this->cloudiness->update_vals(cloudiness);
  this->relative_humidity->update_vals(relative_humidity);
  this->dew_point->update_vals(dew_point);
  int header_collected = https.headers();
#ifdef DEBUG_WEATHER
  Serial.print("Collected ");
  Serial.print(header_collected);
  Serial.println(" headers:");
#endif
  if (header_collected == 2)
  {
    this->last_modified = https.header("last-modified");
    String expires = https.header("expires");
    const char *expires_c = expires.c_str();
    char *end =
        strptime(expires_c, "%a, %d %b %Y %H:%M:%S GMT", this->expired_time);
#ifdef DEBUG_WEATHER
    if ((end == NULL) || end != "\0")
    {
      Serial.print("Found remaining char: ");
      Serial.println(end);
    }
#endif

#ifdef DEBUG_WEATHER
    Serial.print("last-modified: ");
    Serial.println(this->last_modified);

    Serial.print("expires: ");
    Serial.println(expires);
    Serial.println(this->expired_time->tm_hour);
    Serial.println(this->expired_time->tm_min);
#endif
  }
  https.end();
  delete[] temps;
  delete[] precipitation;
  delete[] wind_speeds;
  delete[] wind_directions;
  delete[] air_pressure;
  delete[] cloudiness;
  delete[] relative_humidity;
  delete[] dew_point;
}

bool is_leap_year(int year)
{
  if (year % 4 != 0)
  {
    return false;
  }
  else if (year % 100 != 0)
  {
    return true;
  }
  else if (year % 400 != 0)
  {
    return false;
  }
  else
  {
    return true;
  }
}

bool WeatherRFP::is_expired(void)
{
  getLocalTime(this->local_time);
  int8_t day_add = 0;
  int8_t hour = this->expired_time->tm_hour + this->utc_offset +
                (int8_t)this->daylight_saving;
  int year = this->expired_time->tm_year;
  if (hour > 23)
  {
    day_add = 1;
    hour = (hour - 24);
  }
  else if (hour < 0)
  {
    day_add = -1;
    hour = 24 + hour;
  }
  int days_in_year = is_leap_year(year) ? 366 : 365;
  if (this->expired_time->tm_yday + day_add > days_in_year)
  {
    year += 1;
  }

  // Check if the expiration year is less than the current year or if the
  // expiration day of the year (with day adjustment) is less than the current
  // day of the year
  if ((year < this->local_time->tm_year) ||
      ((this->expired_time->tm_yday + day_add) < this->local_time->tm_yday))
  {
    return true;
  }

  // Check if the expiration hour is less than the current hour
  if (hour < this->local_time->tm_hour)
  {
    return true;
  }
  if ((hour == this->local_time->tm_hour) &&
      (this->expired_time->tm_min < this->local_time->tm_min))
  {
    return true;
  }
  return false;
}

WeatherDataRFP *WeatherRFP::get_temperature() { return this->temperature; }
WeatherDataRFP *WeatherRFP::get_precipitation() { return this->precipitation; }
WeatherDataRFP *WeatherRFP::get_air_pressure() { return this->air_pressure; }
WeatherDataRFP *WeatherRFP::get_relative_humidity() { return this->relative_humidity; }
WeatherDataRFP *WeatherRFP::get_wind_speeds() { return this->wind_speeds; }
WeatherDataRFP *WeatherRFP::get_wind_direction() { return this->wind_direction; }
WeatherDataRFP *WeatherRFP::get_cloudiness() { return this->cloudiness; }
WeatherDataRFP *WeatherRFP::get_dew_point() { return this->dew_point; }
void WeatherRFP::setExpiredTime(tm *time) { this->expired_time = time; }
tm *WeatherRFP::getExpiredTime() { return this->expired_time; }
uint8_t WeatherRFP::get_num_hours() { return this->num_hours; }
void WeatherRFP::set_symbol_code_next_1h(String symbol) { this->symbol_code_next_1h = symbol; }
void WeatherRFP::set_symbol_code_next_6h(String symbol) { this->symbol_code_next_6h = symbol; }
void WeatherRFP::set_symbol_code_next_12h(String symbol) { this->symbol_code_next_12h = symbol; }
String WeatherRFP::get_symbol_code_next_1h() { return this->symbol_code_next_1h; }
String WeatherRFP::get_symbol_code_next_6h() { return this->symbol_code_next_6h; }
String WeatherRFP::get_symbol_code_next_12h() { return this->symbol_code_next_12h; }
