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
#include <weather.hpp>
#include <WiFi.h>

Weather *weather_status;

void setup() {

  Serial.begin();

  // get connected
  ...

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
  Serial.print(weather_status->get_temperature()->get_current());
  Serial.println("°C);

  Serial.print("Maximum temperature next 24 hours: ");
  Serial.print(weather_status->get_temperature()->get_maximum());
  Serial.println("°C);

  Serial.print("Maximum temperature next 24 hours: ");
  Serial.print(weather_status->get_temperature()->get_minimum());
  Serial.println("°C);

  Serial.print("Mean temperature next 24 hours: ");
  Serial.print(weather_status->get_temperature()->get_mean());
  Serial.println("°C);

}
```

## Available data
## Weather Data Stored

| Data Type         | Unit   |
|-------------------|--------|
| Temperature       | °C     |
| Precipitation     | mm     |
| Dew point         | °C     |
| Air pressure      | hPa    |
| Wind speed        | m/s    |
| Wind direction    | deg    |
| Cloudiness        | %      |
| Relative humidity | %      |

## Classes - Code Documentation
### Weather Class – Public Interface

The `Weather` class provides an interface to retrieve and manage weather data from the [api.met.no weather API](https://api.met.no/weatherapi/locationforecast/2.0/complete).

#### Public Methods

| **Return Type**         | _Function_                                  | Description                                                        |
|-------------------------|---------------------------------------------|--------------------------------------------------------------------|
|                         | _Weather(double latitude, double longitude, String user_agent)_ | Constructor to initialize with latitude, longitude, and user agent.|
|                         | _Weather(double latitude, double longitude, uint16_t altitude, String user_agent)_ | Constructor with latitude, longitude, altitude, and user agent.    |
|                         | _~Weather()_                                | Destructor.                                                        |
| **bool**                | _is_expired(void)_                          | Checks if the weather data is expired.                             |
| **void**                | _update_data(void)_                         | Updates the weather data from the API.                             |
| **void**                | _update_location(double latitude, double longitude)_ | Updates the location (latitude and longitude).              |
| **void**                | _update_location(double latitude, double longitude, uint16_t altitude)_ | Updates the location with latitude, longitude, and altitude. |
| **void**                | _set_utc_offset(int8_t utf_offset)_         | Sets the UTC offset for the location.                              |
| **void**                | _set_daylight_saving(bool daylight_saving)_ | Sets the daylight saving status.                                   |
| **WeatherData\***       | _get_temperature()_                         | Returns a pointer to temperature data.                             |
| **WeatherData\***       | _get_precipitation()_                       | Returns a pointer to precipitation data.                           |
| **WeatherData\***       | _get_wind_speeds()_                         | Returns a pointer to wind speed data.                              |
| **WeatherData\***       | _get_wind_direction()_                      | Returns a pointer to wind direction data.                          |
| **WeatherData\***       | _get_air_pressure()_                        | Returns a pointer to air pressure data.                            |
| **WeatherData\***       | _get_cloudiness()_                          | Returns a pointer to cloudiness data.                              |
| **WeatherData\***       | _get_relative_humidity()_                   | Returns a pointer to relative humidity data.                       |
| **WeatherData\***       | _get_dew_point()_                           | Returns a pointer to dew point data.                               |
| **tm\***                | _getExpiredTime()_                          | Returns a pointer to the expiration time structure.                |
| **String**              | _get_symbol_code_next_1h()_                 | Returns the symbol code for the next 1 hour.                       |
| **String**              | _get_symbol_code_next_6h()_                 | Returns the symbol code for the next 6 hours.                      |
| **String**              | _get_symbol_code_next_12h()_                | Returns the symbol code for the next 12 hours.                     |

> **Note:**  
> The Weather class does not expose public data members; all interaction is through its public methods.

### WeatherData Class – Public Interface

The `WeatherData` class represents weather parameter data for a set of hours, providing statistical and value access methods.

#### Public Methods

| **Return Type** | _Function_                              | Description                                                  |
|-----------------|-----------------------------------------|--------------------------------------------------------------|
|                 | _WeatherData(uint8_t num_hours)_        | Constructor that initializes with the number of hours.        |
|                 | _WeatherData(double *vals)_             | Constructor that initializes with a pointer to values array.  |
|                 | _~WeatherData()_                        | Destructor.                                                  |
| **void**        | _update_vals(double *vals)_             | Updates the internal values array.                            |
| **double**      | _get_minimum()_                         | Returns the minimum value.                                    |
| **double**      | _get_maximum()_                         | Returns the maximum value.                                    |
| **double**      | _get_mean()_                            | Returns the mean value.                                       |
| **double**      | _get_std()_                             | Returns the standard deviation.                               |
| **double**      | _get_variance()_                        | Returns the variance.                                         |
| **double**      | _get_val_at_hour(uint8_t hour)_         | Returns the value at the specified hour.                      |
| **double**      | _get_current()_                         | Returns the value for the current hour.                       |

> **Note:**  
> The WeatherData class does not expose public data members; all access is through its public methods.

## Debugging Tips

- **Enable Debug Output:**  
  Uncomment or define `#define DEBUG_WEATHER` at the top of your source files to enable debug print statements throughout the code. This will give you more insight into what your code is doing at runtime.

- **Use Serial Monitor:**  
  Make sure to initialize `Serial.begin(115200);` in your `setup()` function. Use `Serial.print()` and `Serial.println()` to output variable values and checkpoints.

- **Check WiFi Connectivity:**  
  Ensure the ESP32 is connected to WiFi before making HTTP requests. Print the WiFi status and IP address for confirmation.


---

> **Tip:**  
> If you encounter persistent issues, try running a minimal example sketch that only tests the failing module or function.
