#ifndef WEATHER_DATA_HPP
#define WEATHER_DATA_HPP
#include <Arduino.h>
class WeatherData
{

private:
  double *vals;
  double minimum;
  double maxmimum;
  double mean;
  double variance;
  uint8_t num_hours;

public:
  WeatherData(uint8_t num_hours);
  WeatherData(double *vals);
  ~WeatherData();
  void update_vals(double *vals);
  double get_minimum();
  double get_maximum();
  double get_mean();
  double get_std();
  double get_variance();
  double get_val_at_hour(uint8_t hour);
  double get_current();
};
#endif