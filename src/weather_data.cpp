#include "weather_data.hpp"
#include <math.h>

WeatherData::WeatherData(uint8_t num_hours)
{
  this->vals = new double[num_hours + 1];
  this->minimum = 0;
  this->maxmimum = 0;
  this->mean = 0;
  this->variance = 0;
  this->num_hours = num_hours;
}

double WeatherData::get_minimum()
{
  return this->minimum;
}

double WeatherData::get_maximum()
{
  return this->maxmimum;
}

double WeatherData::get_mean()
{
  return this->mean;
}

double WeatherData::get_variance()
{
  return this->variance;
}

double WeatherData::get_std()
{
  return sqrt(this->variance);
}

double WeatherData::get_val_at_hour(uint8_t hour)
{
  return this->vals[hour];
}

WeatherData::~WeatherData()
{
  delete[] this->vals;
}

void WeatherData::update_vals(double *vals)
{
  double mean = 0.0;
  double minimum = vals[0];
  double maximum = vals[0];
  for (uint8_t i = 0; i < this->num_hours + 1; ++i)
  {
    this->vals[i] = vals[i];
    if (i > 0)
    {
      mean += vals[i];
      if (vals[i] < minimum)
      {
        minimum = vals[i];
      }
      if (vals[i] > maximum)
      {
        maximum = vals[i];
      }
    }
  }
  this->mean = mean / this->num_hours;
  this->maxmimum = maximum;
  this->minimum = minimum;
  if (this->num_hours > 1)
  {
    double var = 0.0;
    for (int i = 1; i < this->num_hours + 1; ++i)
    {
      var += pow(vals[i] - this->mean, 2);
    }

    var /= (this->num_hours - 1);
    this->variance = var;
  }
}

double WeatherData::get_current()
{
  return this->vals[0];
}