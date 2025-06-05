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
  WeatherDataRFP(uint8_t num_hours, float factor);
  ~WeatherDataRFP();
  void update_vals(float *vals);
  float get_minimum();
  float get_maximum();
  float get_mean();
  float get_std();
  float get_variance();
  float get_val_at_hour(uint8_t hour);
  int16_t get_val_at_hour_raw(uint8_t hour);
  float get_current();
  int16_t get_current_raw();
  float get_factor();
};
#endif