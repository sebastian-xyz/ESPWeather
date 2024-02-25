#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "weather.hpp"
#include "math.h"
#include <WiFiClientSecure.h>

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

Weather::Weather(double latitude, double longitude)
{
  this->longitude = longitude;
  this->latitude = latitude;
  this->altitude = 0;
  this->local_time = new tm;
  this->expired_time = new tm;
  this->temperature = new WeatherData(this->num_hours);
  this->precipitation = new WeatherData(this->num_hours);
  this->wind_speeds = new WeatherData(this->num_hours);
  this->wind_direction = new WeatherData(this->num_hours);
  this->air_pressure = new WeatherData(this->num_hours);
  this->cloudiness = new WeatherData(this->num_hours);
  this->relative_humidity = new WeatherData(this->num_hours);
  this->last_modified = "";
  this->utc_offset = 0;
}
Weather::Weather(double latitude, double longitude, uint16_t altitude)
{
  this->longitude = longitude;
  this->latitude = latitude;
  this->altitude = altitude;
  this->local_time = new tm;
  this->expired_time = new tm;
  this->temperature = new WeatherData(this->num_hours);
  this->precipitation = new WeatherData(this->num_hours);
  this->wind_speeds = new WeatherData(this->num_hours);
  this->wind_direction = new WeatherData(this->num_hours);
  this->air_pressure = new WeatherData(this->num_hours);
  this->cloudiness = new WeatherData(this->num_hours);
  this->relative_humidity = new WeatherData(this->num_hours);
  this->last_modified = "";
  this->utc_offset = 0;
}

void Weather::set_utc_offset(int8_t utc_offset)
{
  this->utc_offset = utc_offset;
}

Weather::~Weather()
{
  delete this->local_time;
  delete this->expired_time;
}

void Weather::update_location(double longitude, double latitude)
{
  this->longitude = longitude;
  this->latitude = latitude;
}
void Weather::update_location(double longitude, double latitude, double altitude)
{
  this->longitude = longitude;
  this->latitude = latitude;
  this->altitude = altitude;
}

void Weather::update_data(void)
{
  WiFiClientSecure *client = new WiFiClientSecure;
  client->setCACert(root_cert);
  HTTPClient https;
  https.useHTTP10(true);
  char *buffer = new char[512];
  sprintf(buffer, "?lat=%.2f&lon=%.2f&altitude=%d", this->latitude, this->longitude, this->altitude);
  String url = (String)this->url + (String)buffer;
  https.begin(*client, url.c_str());
  https.setUserAgent("Mozilla/5.0 (Linux x86_64)");
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
    filter["properties"]["timeseries"][i]["data"]["instant"]["air_temperature"] = true;
    filter["properties"]["timeseries"][i]["data"]["instant"]["precipitation_amount"] = true;
    filter["properties"]["timeseries"][i]["data"]["instant"]["wind_speed"] = true;
    filter["properties"]["timeseries"][i]["data"]["instant"]["wind_from_direction"] = true;
    filter["properties"]["timeseries"][i]["data"]["instant"]["air_pressure_at_sea_level"] = true;
    filter["properties"]["timeseries"][i]["data"]["instant"]["cloud_area_fraction"] = true;
    filter["properties"]["timeseries"][i]["data"]["instant"]["relative_humidity"] = true;
  }

  int httpResponseCode = https.GET();
  String payload = "{}";
  if (httpResponseCode > 0)
  {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    if ((!this->last_modified.isEmpty()) && httpResponseCode == 304)
    {
      Serial.println("Data unchanged. Nothing todo");
      return;
    }
  }
  else
  {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, https.getStream(), DeserializationOption::Filter(filter));

  Serial.println("Starting deserialization");
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
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
  double *temps = new double[this->num_hours + 1];
  double *precipitation = new double[this->num_hours + 1];
  double *wind_speeds = new double[this->num_hours + 1];
  double *wind_directions = new double[this->num_hours + 1];
  double *air_pressure = new double[this->num_hours + 1];
  double *cloudiness = new double[this->num_hours + 1];
  double *relative_humidity = new double[this->num_hours + 1];
  JsonObject current_timeseries_data;
  JsonObject current_timeseries_details;
  for (uint8_t i = 0; i < this->num_hours + 1; ++i)
  {
    current_timeseries_data = timeseries[i]["data"];
    current_timeseries_details = current_timeseries_data["instant"]["details"];
    temps[i] = current_timeseries_details["air_temperature"];
    precipitation[i] = current_timeseries_details["precipitation_amount"];
    wind_speeds[i] = current_timeseries_details["wind_speed"];
    wind_directions[i] = current_timeseries_details["wind_from_direction"];
    air_pressure[i] = current_timeseries_details["air_pressure_at_sea_level"];
    cloudiness[i] = current_timeseries_details["cloud_area_fraction"];
    relative_humidity[i] = current_timeseries_details["relative_humidity"];
  }
  // Free resources
  https.end();
  this->temperature->update_vals(temps);
  this->precipitation->update_vals(precipitation);
  this->wind_speeds->update_vals(wind_speeds);
  this->wind_direction->update_vals(wind_directions);
  this->air_pressure->update_vals(air_pressure);
  this->cloudiness->update_vals(cloudiness);
  this->relative_humidity->update_vals(relative_humidity);
  int header_collected = https.headers();
  Serial.print("Collected ");
  Serial.print(header_collected);
  Serial.println("headers:");
  if (header_collected == 2)
  {
    this->last_modified = https.header("last-modified");
    String expires = https.header("expires");

    Serial.print("last-modified: ");
    Serial.println(this->last_modified);

    Serial.print("expires: ");
    Serial.println(expires);
    const char *expires_c = expires.c_str();
    Serial.println(strptime(expires_c, "%a, %d %b %Y %H:%M:%S GMT", expired_time));
    Serial.println(this->expired_time->tm_hour);
    Serial.println(this->expired_time->tm_min);
  }
}

bool Weather::is_expired(void)
{
  getLocalTime(this->local_time);
  int8_t day_add = 0;
  int8_t hour = this->expired_time->tm_hour + utc_offset;
  int year = this->expired_time->tm_year;
  if (hour > 23)
  {
    day_add = 1;
    hour = (this->expired_time->tm_hour + this->utc_offset - 24);
  }
  else if (hour < 0)
  {
    day_add = -1;
    hour = 24 - (this->expired_time->tm_hour + this->utc_offset);
  }
  if (this->expired_time->tm_yday + day_add > 365)
  {
    year += 1;
  }

  if ((year < this->local_time->tm_year) || ((this->expired_time->tm_yday + day_add) < this->local_time->tm_yday))
  {
    return true;
  }
  if (hour < this->local_time->tm_hour)
  {
    return true;
  }
  if ((hour == this->local_time->tm_hour) && (this->expired_time->tm_min < this->local_time->tm_min))
  {
    return true;
  }
  return false;
}

WeatherData *Weather::get_temperature()
{
  return this->temperature;
}
WeatherData *Weather::get_precipitation()
{
  return this->precipitation;
}
WeatherData *Weather::get_air_pressure()
{
  return this->air_pressure;
}

WeatherData *Weather::get_relative_humidity()
{
  return this->relative_humidity;
}

WeatherData *Weather::get_wind_speeds()
{
  return this->wind_speeds;
}

WeatherData *Weather::get_wind_direction()
{
  return this->wind_direction;
}

WeatherData *Weather::get_cloudiness()
{
  return this->cloudiness;
}