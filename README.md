# ESPWeather

Get location forecast data from the [API](https://api.met.no/weatherapi/locationforecast/2.0/documentation) of MET Norway.

## Getting Started

`platformio.ini`
```ini
[env:esp32]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps = 
	https://github.com/sebastian-xyz/ESPWeather.git
```


Example sketch:
```cpp
#include <Arduino.h>
#include <WiFiManager.h>
#include <weather.hpp>
#include <WiFi.h>

Weather *weather_status;

void setup() {

  // get connected
  WiFiManager wifi_manager;
  wifi_manager.autoConnect("ESP-AP");

  // get local time set
  struct tm local;
  configTzTime(TIMEZONE, "pool.ntp.org");
  getLocalTime(&local, 10000);

  weather_status = new Weather(LONGITUDE, LATITUDE, ALTITUDE);
  weather_status->set_utc_offset(1);

  if (WiFi.isConnected()) {
    weather_status->update_data();
  }

}

void loop() {

  if (weather_status->is_expired()) {
    weather_status->update_data();
  }
  Serial.print("Current temperature: ");
  Serial.print(weather_status->get_temperature->get_current());
  Serial.println("°C);

  Serial.print("Maximum temperature next 24 hours: ");
  Serial.print(weather_status->get_temperature->get_maximum());
  Serial.println("°C);

  Serial.print("Maximum temperature next 24 hours: ");
  Serial.print(weather_status->get_temperature->get_minimum());
  Serial.println("°C);

  Serial.print("Mean temperature next 24 hours: ");
  Serial.print(weather_status->get_temperature->get_mean());
  Serial.println("°C);

}
```

## Available data

|Temperature| Unit |
| --- | --- |
| Temperature | `°C` |
| Precipitation | `mm` |
| Dew point | `°C` |
| Air pressure | `hPa` |
| Wind speed | `m/s` |
| Wind direction | `deg` |
| Cloudiness | `%` |
| Relative humidity | `%` |


