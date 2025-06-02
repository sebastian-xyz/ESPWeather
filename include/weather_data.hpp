#ifndef WEATHER_DATA_HPP
#define WEATHER_DATA_HPP
#include <Arduino.h>
class WeatherData
{

private:
  float *vals;
  float minimum;
  float maxmimum;
  float mean;
  float variance;
  uint8_t num_hours;

public:
  WeatherData(uint8_t num_hours);
  WeatherData(float *vals);
  ~WeatherData();
  void update_vals(float *vals);
  float get_minimum();
  float get_maximum();
  float get_mean();
  float get_std();
  float get_variance();
  float get_val_at_hour(uint8_t hour);
  float get_current();
};
#endif