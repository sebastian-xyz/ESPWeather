#ifndef WEATHER_HPP
#define WEATHER_HPP

#include <Arduino.h>
#include "weather_data.hpp"

#define NUM_HOURS 24

class Weather
{

private:
  String last_modified;
  tm *expired_time;
  tm *local_time;
  double longitude;
  double latitude;
  uint16_t altitude;
  int8_t utc_offset;
  const char *url = "https://api.met.no/weatherapi/locationforecast/2.0/complete";
  const uint8_t num_hours = NUM_HOURS;
  WeatherData *dew_point;
  WeatherData *temperature;
  WeatherData *precipitation;
  WeatherData *wind_speeds;
  WeatherData *wind_direction;
  WeatherData *air_pressure;
  WeatherData *cloudiness;
  WeatherData *relative_humidity;
  bool daylight_saving;

public:
  Weather(double latitude, double longitude);
  Weather(double latitude, double longitude, uint16_t altitude);
  ~Weather();
  bool is_expired(void);
  void update_data(void);
  void update_location(double latitude, double longitude);
  void update_location(double latitude, double longitude, uint16_t altitude);
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
};

#endif
