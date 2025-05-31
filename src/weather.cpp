#include "weather.hpp"
#include "math.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <memory>
#include <time.h>

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
  this->user_agent = ESPWeatherUserAgent;
  this->longitude = longitude;
  this->latitude = latitude;
  this->altitude = 0;
  this->local_time = new tm;
  this->expired_time = new tm;
  this->temperature = new WeatherData(this->num_hours);
  this->dew_point = new WeatherData(this->num_hours);
  this->precipitation = new WeatherData(this->num_hours);
  this->wind_speeds = new WeatherData(this->num_hours);
  this->wind_direction = new WeatherData(this->num_hours);
  this->air_pressure = new WeatherData(this->num_hours);
  this->cloudiness = new WeatherData(this->num_hours);
  this->relative_humidity = new WeatherData(this->num_hours);
  this->last_modified = "";
  this->symbol_code_next_1h = "";
  this->symbol_code_next_12h = "";
  this->symbol_code_next_6h = "";
  this->utc_offset = 0;
  this->daylight_saving = false;
}
Weather::Weather(double latitude, double longitude, uint16_t altitude)
{
  this->user_agent = ESPWeatherUserAgent;
  this->longitude = longitude;
  this->latitude = latitude;
  this->altitude = altitude;
  this->local_time = new tm;
  this->expired_time = new tm;
  this->temperature = new WeatherData(this->num_hours);
  this->dew_point = new WeatherData(this->num_hours);
  this->precipitation = new WeatherData(this->num_hours);
  this->wind_speeds = new WeatherData(this->num_hours);
  this->wind_direction = new WeatherData(this->num_hours);
  this->air_pressure = new WeatherData(this->num_hours);
  this->cloudiness = new WeatherData(this->num_hours);
  this->relative_humidity = new WeatherData(this->num_hours);
  this->last_modified = "";
  this->symbol_code_next_1h = "";
  this->symbol_code_next_12h = "";
  this->symbol_code_next_6h = "";
  this->daylight_saving = false;
  this->utc_offset = 0;
}

void Weather::set_daylight_saving(bool daylight_saving)
{
  this->daylight_saving = daylight_saving;
}

void Weather::set_utc_offset(int8_t utc_offset)
{
  this->utc_offset = utc_offset;
}

Weather::~Weather()
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

void Weather::update_location(double latitude, double longitude)
{
  this->longitude = longitude;
  this->latitude = latitude;
}
void Weather::update_location(double latitude, double longitude,
                              uint16_t altitude)
{
  this->longitude = longitude;
  this->latitude = latitude;
  this->altitude = altitude;
}

void Weather::update_data(void)
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
  DeserializationError error = deserializeJson(
      doc, https.getStream(), DeserializationOption::Filter(filter));

#ifdef DEBUG_WEATHER
  Serial.println("Starting deserialization");
#endif
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
  double *temps = new double[this->num_hours + 1];
  double *precipitation = new double[this->num_hours + 1];
  double *wind_speeds = new double[this->num_hours + 1];
  double *wind_directions = new double[this->num_hours + 1];
  double *air_pressure = new double[this->num_hours + 1];
  double *cloudiness = new double[this->num_hours + 1];
  double *relative_humidity = new double[this->num_hours + 1];
  double *dew_point = new double[this->num_hours + 1];
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

bool Weather::is_expired(void)
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

WeatherData *Weather::get_temperature() { return this->temperature; }
WeatherData *Weather::get_precipitation() { return this->precipitation; }
WeatherData *Weather::get_air_pressure() { return this->air_pressure; }

WeatherData *Weather::get_relative_humidity()
{
  return this->relative_humidity;
}

WeatherData *Weather::get_wind_speeds() { return this->wind_speeds; }

WeatherData *Weather::get_wind_direction() { return this->wind_direction; }

WeatherData *Weather::get_cloudiness() { return this->cloudiness; }

WeatherData *Weather::get_dew_point() { return this->dew_point; }

tm *Weather::getExpiredTime() { return this->expired_time; }
String Weather::get_symbol_code_next_1h() { return this->symbol_code_next_1h; }
String Weather::get_symbol_code_next_12h()
{
  return this->symbol_code_next_12h;
}
String Weather::get_symbol_code_next_6h() { return this->symbol_code_next_6h; }
