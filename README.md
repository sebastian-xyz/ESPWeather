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

---

## About the Reduced FootPrint (RFP) Classes

**ESPWeather** provides two main class variants for weather data:

- **Standard classes:** `Weather`, `WeatherData`  
  These use `float` arrays for storing weather data and are suitable for most ESP32 applications.

- **Reduced FootPrint (RFP) classes:** `WeatherRFP`, `WeatherDataRFP`  
  These are designed for environments with limited memory.  
  - Data is stored in `int16_t` arrays and scaled by a `factor` (see the `ESPWeatherRFPFactor*` defines in the code).
  - The RFP classes use the same API as the standard classes, so you can switch between them with minimal code changes.
  - To use the RFP classes, include `weather_rfp.hpp` instead of the standard headers and replace `Weather` with `WeatherRFP.
  - Additionally `WeatherDataRFP` provides `int16_6 get_current_raw()` and `int16_t get_val_at_hour_raw(uint8_t hour)` to gain direct access to the stored values

**RFP Defines and Default Values:**

| Define                                 | Default Value | Description                              |
|-----------------------------------------|--------------|------------------------------------------|
| `ESPWeatherRFPFactorTemperature`        | 500.0f      | Temperature (°C × 500)                  |
| `ESPWeatherRFPFactorDewPoint`           | 500.0f      | Dew point (°C × 500)                    |
| `ESPWeatherRFPFactorPrecipitation`      | 1000.0f      | Precipitation (mm × 1000)                |
| `ESPWeatherRFPFactorWindSpeed`          | 100.0f       | Wind speed (m/s × 100)                   |
| `ESPWeatherRFPFactorWindDirection`      | 50.0f       | Wind direction (degrees × 100)           |
| `ESPWeatherRFPFactorAirPressure`        | 10.0f        | Air pressure (hPa × 10)                  |
| `ESPWeatherRFPFactorCloudiness`         | 100.0f      | Cloudiness (% × 100)                    |
| `ESPWeatherRFPFactorRelativeHumidity`   | 100.0f      | Relative humidity (% × 100)             |

These defines set the scaling factors for each weather parameter, allowing the library to store values as scaled integers and save RAM. For example, a temperature of 23.456°C is stored as `11728` when using a factor of 500.

> **How to change the scaling factors:**  
> To override any of these default values, simply `#define` the desired value **before** including `weather_rfp.hpp` in your code.  
> For example:
> ```cpp
> #define ESPWeatherRFPFactorTemperature 100.0f
> #include <weather_rfp.hpp>
> ```
---

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
|                         | _Weather(float latitude, float longitude)_  | Constructor to initialize with latitude and longitude.             |
|                         | _Weather(float latitude, float longitude, uint16_t altitude)_ | Constructor with latitude, longitude, and altitude.    |
|                         | _Weather(uint8_t num_hours, float latitude, float longitude)_ | Constructor with hours, latitude, longitude.         |
|                         | _Weather(uint8_t num_hours, float latitude, float longitude, uint16_t altitude)_ | Constructor with hours, latitude, longitude, altitude. |
|                         | _~Weather()_                                | Destructor.                                                        |
| **bool**                | _is_expired(void)_                          | Checks if the weather data is expired.                             |
| **void**                | _update_data(void)_                         | Updates the weather data from the API.                             |
| **void**                | _update_location(float latitude, float longitude)_ | Updates the location (latitude and longitude).              |
| **void**                | _update_location(float latitude, float longitude, uint16_t altitude)_ | Updates the location with latitude, longitude, and altitude. |
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
| **void**                | _setExpiredTime(tm *time)_                  | Sets a pointer to the expiration time structure.                   |
| **tm\***                | _getExpiredTime()_                          | Returns a pointer to the expiration time structure.                |
| **void**                | _set_symbol_code_next_1h(String symbol)     | Sets a stored symbol code for the next 1 hour.                     |
| **void**                | _set_symbol_code_next_6h(String symbol)     | Sets a stored symbol code for the next 6 hour.                     |
| **void**                | _set_symbol_code_next_12h(String symbol)    | Sets a stored symbol code for the next 12 hours.                   |
| **String**              | _get_symbol_code_next_1h()_                 | Returns the symbol code for the next 1 hour.                       |
| **String**              | _get_symbol_code_next_6h()_                 | Returns the symbol code for the next 6 hours.                      |
| **String**              | _get_symbol_code_next_12h()_                | Returns the symbol code for the next 12 hours.                     |

> **Note:**  
> The Weather class does not expose public data members; all interaction is through its public methods.

### WeatherData Class – Public Interface

The `WeatherData` class represents weather parameter data for a set of hours, providing statistical and value access methods.

#### Public Methods

| **Return Type** | _Function_                              | Description                                                   |
|-----------------|-----------------------------------------|---------------------------------------------------------------|
|                 | _WeatherData(uint8_t num_hours)_        | Constructor that initializes with the number of hours.        |
|                 | _WeatherData(float *vals)_              | Constructor that initializes with a pointer to values array.  |
|                 | _~WeatherData()_                        | Destructor.                                                   |
| **void**        | _update_vals(float *vals)_              | Updates the internal values array.                            |
| **void**        | _update_vals(int16_t *vals)_            | Updates the internal values array.                            |
| **float**       | _get_minimum()_                         | Returns the minimum value.                                    |
| **float**       | _get_maximum()_                         | Returns the maximum value.                                    |
| **float**       | _get_mean()_                            | Returns the mean value.                                       |
| **float**       | _get_std()_                             | Returns the standard deviation.                               |
| **float**       | _get_variance()_                        | Returns the variance.                                         |
| **float**       | _get_val_at_hour(uint8_t hour)_         | Returns the value at the specified hour.                      |
| **int16_t**     | _get_val_at_hour_raw(uint8_t hour)_     | Returns the value at the specified hour.                      |
| **float**       | _get_current()_                         | Returns the value for the current hour.                       |
| **int16_t**     | _get_current_raw()_                     | Returns the value for the current hour.                       |

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
