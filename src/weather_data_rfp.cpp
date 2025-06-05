#include "weather_data_rfp.hpp"
#include <math.h>

WeatherDataRFP::WeatherDataRFP(uint8_t num_hours, float factor)
{
  this->vals = new int16_t[num_hours + 1];
  this->minimum = 0;
  this->maxmimum = 0;
  this->mean = 0;
  this->variance = 0;
  this->num_hours = num_hours;
  this->factor = factor;
}

float WeatherDataRFP::get_minimum()
{
  return static_cast<float>(this->minimum) / this->factor;
}

float WeatherDataRFP::get_maximum()
{
  return static_cast<float>(this->maxmimum) / this->factor;
}

float WeatherDataRFP::get_mean()
{
  return static_cast<float>(this->mean) / this->factor;
}

float WeatherDataRFP::get_variance()
{
  return static_cast<float>(this->variance) / this->factor;
}

float WeatherDataRFP::get_std()
{
  return sqrt(this->get_variance());
}

float WeatherDataRFP::get_val_at_hour(uint8_t hour)
{
  return static_cast<float>(this->vals[hour]) / this->factor;
}

int16_t WeatherDataRFP::get_val_at_hour_raw(uint8_t hour)
{
  return this->vals[hour];
}

WeatherDataRFP::~WeatherDataRFP()
{
  delete[] this->vals;
}

void WeatherDataRFP::update_vals(float *vals)
{
  float mean = 0.0;
  float minimum = vals[0];
  float maximum = vals[0];
  for (uint8_t i = 0; i < this->num_hours + 1; ++i)
  {
    this->vals[i] = static_cast<int16_t>(vals[i] * this->factor);
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
  this->mean = static_cast<int16_t>(mean / this->num_hours * this->factor);
  this->maxmimum = static_cast<int16_t>(maximum * this->factor);
  this->minimum = static_cast<int16_t>(minimum * this->factor);
  if (this->num_hours > 1)
  {
    float var = 0.0;
    for (int i = 1; i < this->num_hours + 1; ++i)
    {
      var += pow(vals[i] - this->mean, 2);
    }

    var /= (this->num_hours - 1);
    this->variance = static_cast<int16_t>(var * this->factor);
  }
}

float WeatherDataRFP::get_current()
{
  return static_cast<float>(this->vals[0]) / this->factor;
}
int16_t WeatherDataRFP::get_current_raw()
{
  return this->vals[0];
}
float WeatherDataRFP::get_factor()
{
  return this->factor;
}
