#ifndef WEATHER_DATA_RFP_HPP
#define WEATHER_DATA_RFP_HPP
#include <Arduino.h>
class WeatherDataRFP
{

private:
  int16_t *vals;
  int16_t minimum;
  int16_t maxmimum;
  int16_t mean;
  int16_t variance;
  float factor;
  uint8_t num_hours;

public:
  WeatherData(uint8_t num_hours, float factor);
  WeatherData(float *vals, float factor);
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