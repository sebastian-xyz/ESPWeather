#ifndef WEATHER_HPP
#define WEATHER_HPP

#include <Arduino.h>
#include "weather_data.hpp"

#ifndef ESPWeatherNumHours
#define ESPWeatherNumHours 24
#endif
#ifndef ESPWeatherUserAgent
#define ESPWeatherUserAgent "github.com/sebastian-xyz/ESPWeather/v1.1"
#endif

class Weather
{

private:
  String user_agent;
  String last_modified;
  tm *expired_time;
  tm *local_time;
  float longitude;
  float latitude;
  uint16_t altitude;
  int8_t utc_offset;
  const char *url = "https://api.met.no/weatherapi/locationforecast/2.0/complete";
  const uint8_t num_hours;
  WeatherData *dew_point;
  WeatherData *temperature;
  WeatherData *precipitation;
  WeatherData *wind_speeds;
  WeatherData *wind_direction;
  WeatherData *air_pressure;
  WeatherData *cloudiness;
  WeatherData *relative_humidity;
  String symbol_code_next_1h;
  String symbol_code_next_6h;
  String symbol_code_next_12h;
  bool daylight_saving;

public:
  Weather(float latitude, float longitude);
  Weather(float latitude, float longitude, uint16_t altitude);
  Weather(uint8_t num_hours, float latitude, float longitude);
  Weather(uint8_t num_hours, float latitude, float longitude, uint16_t altitude);
  Weather(float latitude, float longitude, uint16_t altitude);
  ~Weather();
  bool is_expired(void);
  void update_data(void);
  void update_location(float latitude, float longitude);
  void update_location(float latitude, float longitude, uint16_t altitude);
  void set_utc_offset(int8_t utf_offset);
  void set_daylight_saving(bool daylight_saving);
  WeatherData *get_temperature();
  WeatherData *get_precipitation();
  WeatherData *get_wind_speeds();
  WeatherData *get_wind_direction();
  WeatherData *get_air_pressure();
  WeatherData *get_cloudiness();
  WeatherData *get_relative_humidity();
  WeatherData *get_dew_point();
  tm *getExpiredTime();
  String get_symbol_code_next_1h();
  String get_symbol_code_next_6h();
  String get_symbol_code_next_12h();
};

#endif
