#ifndef WEATHER_RFP_HPP
#define WEATHER_RFP_HPP

#include <Arduino.h>
#include "weather_data_rfp.hpp"
#include "weather_defines.hpp"

#ifndef ESPWeatherRFPFactorTemperature
#define ESPWeatherRFPFactorTemperature 500.0f
#endif
#ifndef ESPWeatherRFPFactorPrecipitation
#define ESPWeatherRFPFactorPrecipitation 1000.0f
#endif
#ifndef ESPWeatherRFPFactorWindSpeed
#define ESPWeatherRFPFactorWindSpeed 100.0f
#endif
#ifndef ESPWeatherRFPFactorWindDirection
#define ESPWeatherRFPFactorWindDirection 100.0f
#endif
#ifndef ESPWeatherRFPFactorAirPressure
#define ESPWeatherRFPFactorAirPressure 10.0f
#endif
#ifndef ESPWeatherRFPFactorCloudiness
#define ESPWeatherRFPFactorCloudiness 100.0f
#endif
#ifndef ESPWeatherRFPFactorRelativeHumidity
#define ESPWeatherRFPFactorRelativeHumidity 100.0f
#endif
#ifndef ESPWeatherRFPFactorDewPoint
#define ESPWeatherRFPFactorDewPoint 500.0f
#endif

// Weather Class with Reduced FootPrint (RFP)
class WeatherRFP
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
  WeatherDataRFP *dew_point;
  WeatherDataRFP *temperature;
  WeatherDataRFP *precipitation;
  WeatherDataRFP *wind_speeds;
  WeatherDataRFP *wind_direction;
  WeatherDataRFP *air_pressure;
  WeatherDataRFP *cloudiness;
  WeatherDataRFP *relative_humidity;
  String symbol_code_next_1h;
  String symbol_code_next_6h;
  String symbol_code_next_12h;
  bool daylight_saving;

public:
  WeatherRFP(float latitude, float longitude);
  WeatherRFP(float latitude, float longitude, uint16_t altitude);
  WeatherRFP(uint8_t num_hours, float latitude, float longitude);
  WeatherRFP(uint8_t num_hours, float latitude, float longitude, uint16_t altitude);
  WeatherRFP(float latitude, float longitude, uint16_t altitude);
  ~WeatherRFP();
  bool is_expired(void);
  void update_data(void);
  void update_location(float latitude, float longitude);
  void update_location(float latitude, float longitude, uint16_t altitude);
  void set_utc_offset(int8_t utf_offset);
  void set_daylight_saving(bool daylight_saving);
  WeatherDataRFP *get_temperature();
  WeatherDataRFP *get_precipitation();
  WeatherDataRFP *get_wind_speeds();
  WeatherDataRFP *get_wind_direction();
  WeatherDataRFP *get_air_pressure();
  WeatherDataRFP *get_cloudiness();
  WeatherDataRFP *get_relative_humidity();
  WeatherDataRFP *get_dew_point();
  tm *getExpiredTime();
  String get_symbol_code_next_1h();
  String get_symbol_code_next_6h();
  String get_symbol_code_next_12h();
};

#endif
