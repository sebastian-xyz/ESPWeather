/*Using LVGL with Arduino requires some extra steps:
 *Be sure to read the docs here: https://docs.lvgl.io/master/get-started/platforms/arduino.html  */
#include <WiFi.h>
#include <esp_wifi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ElegantOTA.h>
#include <lvgl.h>
#include <TFT_eSPI.h>
#include "lv_conf.h"
#include <demos/lv_demos.h>
#include "CST816S.h"
#include "QMI8658.h"
#include "weather_rfp.hpp"
 
#define BAT_ADC_PIN 1
#define uS_TO_S_FACTOR 1000000ULL  // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP  10           // Time ESP32 will go to sleep (in seconds)

LV_IMG_DECLARE(image1);
LV_IMG_DECLARE(image2);
LV_IMG_DECLARE(rolex);
LV_IMG_DECLARE(stdzeiger);
LV_IMG_DECLARE(minzeiger);
LV_IMG_DECLARE(seczeiger);

lv_obj_t *std_scr;
lv_obj_t *img_scr;
lv_obj_t *clk_scr;
lv_timer_t *clk_timer;
lv_obj_t *gyr_scr;
lv_timer_t *gyr_timer;
lv_timer_t *screen_on_timer;
lv_timer_t *deep_sleep_timer;
lv_meter_indicator_t *indic[3]; 
float scale_acc = 100.0f; // scale factor for accelerometer
float velocity[3]; // Velocity in x, y, z
float distance[3]; // Distance in x, y, z
float angle[3]; // Distance in x, y, z
unsigned int last_time = 0;
static int32_t hour;
static int32_t minute;
static int32_t second;

WebServer server(80);
const boolean changeMAC = false; // Set to true if you want to change the MAC address (e.g. in case of captive portals)
const char *ssid = "undisclosed";
const char *password = "2178981614992766";
// const char *ssid = "Radisson_GUEST";
// const char *ssid = "Airport-Frankfurt";
unsigned int wifi_update = 5000;
unsigned long timer_wifi = 0;
unsigned int time_update = 600000;
unsigned long timer_time = 0;
unsigned int display_timeout = 10000;
unsigned int deep_sleep_timeout = display_timeout + 10000;
boolean deepSleep = false;
tm *local_time;
WeatherRFP *weather_status;
lv_obj_t *weather_scr;
RTC_DATA_ATTR struct tm timeinfo;
RTC_DATA_ATTR struct tm expiredTime;
RTC_DATA_ATTR String weather_1h = "";
RTC_DATA_ATTR String weather_6h = "";
RTC_DATA_ATTR String weather_12h = "";
RTC_DATA_ATTR int16_t temperature[ESPWeatherNumHours+1] = {0};
RTC_DATA_ATTR int16_t humidity[ESPWeatherNumHours+1] = {0};
RTC_DATA_ATTR int16_t pressure[ESPWeatherNumHours+1] = {0};
RTC_DATA_ATTR int16_t wind_speed[ESPWeatherNumHours+1] = {0};
RTC_DATA_ATTR int16_t wind_direction[ESPWeatherNumHours+1] = {0};
RTC_DATA_ATTR int16_t precipitation[ESPWeatherNumHours+1] = {0};
RTC_DATA_ATTR int16_t dew_point[ESPWeatherNumHours+1] = {0};
RTC_DATA_ATTR int16_t cloudiness[ESPWeatherNumHours+1] = {0};



//const float conversion_factor = 3.3f / (1 << 12) * 3;
const float conversion_factor = 13.0f / (1 << 12);

// Set your new MAC Address
uint8_t newMACAddress[] = {0xB8, 0x9A, 0x2A, 0x45, 0xBD, 0x02};



/*To use the built-in examples and demos of LVGL uncomment the includes below respectively.
*You also need to copy `lvgl/examples` to `lvgl/src/examples`. Similarly for the demos `lvgl/demos` to `lvgl/src/demos`.
Note that the `lv_examples` library is for LVGL v7 and you shouldn't install it for this version (since LVGL v8)
as the examples and demos are now part of the main LVGL library. */
#define EXAMPLE_LVGL_TICK_PERIOD_MS    2

/*Change to your screen resolution*/
static const uint16_t screenWidth  = 240;
static const uint16_t screenHeight = 240;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ screenWidth * screenHeight / 10 ];

#define TFT_BACKLIGHT_OFF LOW
boolean backlight_LED_on = true;
TFT_eSPI tft = TFT_eSPI(screenWidth, screenHeight); /* TFT instance */
CST816S touch(6, 7, 13, 5);	// sda, scl, rst, irq

// define the GPIO pin for the interrupt from deep sleep
#define WAKEUP_GPIO GPIO_NUM_5

#if LV_USE_LOG != 0
 /* Serial debugging */
void my_print(const char * buf)
{
  Serial.printf(buf);
  Serial.flush();
}
#endif
 
unsigned long ota_progress_millis = 0;
 
void onOTAStart()
{
  // Log when OTA has started
  Serial.println("OTA update started!");
  // <Add your own code here>
}
 
void onOTAProgress(size_t current, size_t final) 
{
  // Log every 1 second
  if (millis() - ota_progress_millis > 1000) {
    ota_progress_millis = millis();
    Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
  }
}
 
void onOTAEnd(bool success)
{
  // Log when OTA has finished
  if (success) {
    Serial.println("OTA update finished successfully!");
  } else {
    Serial.println("There was an error during OTA update!");
  }
  // <Add your own code here>
}
 
void backlightOff()
{
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, TFT_BACKLIGHT_OFF);
  backlight_LED_on = false;
}
 
void backlightOn()
{
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);
  backlight_LED_on = true;
  lv_timer_reset(screen_on_timer);
  lv_timer_reset(deep_sleep_timer);
}

boolean screenIsOn()
{
  return backlight_LED_on;
}

static void stop_img_cb(lv_event_t * e)
{
//  lv_anim_del();
  backlightOn();
  lv_scr_load(std_scr);
  lv_obj_del(img_scr);
  Serial.println("stop_img_cb: screen deleted");
}

static void stop_gyr_cb(lv_event_t * e)
{
  lv_event_code_t code = lv_event_get_code(e);
  Serial.print("stop_gyr_cb: event ");
  Serial.println(code);
  backlightOn();
  if (code == LV_EVENT_GESTURE)
  {
    lv_scr_load(std_scr);
    lv_timer_del(gyr_timer);
    lv_obj_del(lv_obj_get_child(gyr_scr, 0));
    lv_obj_del(gyr_scr);
    Serial.println("stop_gyr_cb: screen deleted");
  }
}

static void stop_weather_cb(lv_event_t * e)
{
  lv_event_code_t code = lv_event_get_code(e);
  Serial.print("stop_weather_cb: event ");
  Serial.println(code);
  backlightOn();
  if (code == LV_EVENT_GESTURE)
  {
    lv_scr_load(std_scr);
    lv_obj_del(lv_obj_get_child(weather_scr, 0));
    lv_obj_del(weather_scr);
    Serial.println("stop_weather_cb: screen deleted");
  }
}

static void stop_clk_cb(lv_event_t * e)
{
  lv_event_code_t code = lv_event_get_code(e);
  Serial.print("stop_clk_cb: event ");
  Serial.println(code);
  backlightOn();
  if (code == LV_EVENT_GESTURE)
  {
    lv_scr_load(std_scr);
    lv_timer_del(clk_timer);
    lv_obj_del(lv_obj_get_child(clk_scr, 0));
    lv_obj_del(clk_scr);
    Serial.println("stop_clk_cb: screen deleted");
  }
}
    
static void screen_wake_up_cb(lv_event_t * e)
{
  lv_event_code_t code = lv_event_get_code(e);
  backlightOn();
}

static void draw_event_cb(lv_event_t * e)
{
  lv_obj_draw_part_dsc_t * dsc = lv_event_get_draw_part_dsc(e);
  if(dsc->part == LV_PART_ITEMS)
  {
    lv_obj_t * obj = lv_event_get_target(e);
    lv_chart_series_t * ser = lv_chart_get_series_next(obj, NULL);
    uint32_t cnt = lv_chart_get_point_count(obj);
    /*Make older value more transparent*/
    dsc->rect_dsc->bg_opa = (LV_OPA_COVER *  dsc->id) / (cnt - 1);

    /*Make smaller values blue, higher values red*/
    lv_coord_t * x_array = lv_chart_get_x_array(obj, ser);
    lv_coord_t * y_array = lv_chart_get_y_array(obj, ser);
    /*dsc->id is the tells drawing order, but we need the ID of the point being drawn.*/
    uint32_t start_point = lv_chart_get_x_start_point(obj, ser);
    uint32_t p_act = (start_point + dsc->id) % cnt; /*Consider start point to get the index of the array*/
    lv_opa_t x_opa = (x_array[p_act] * LV_OPA_50) / 100;
    lv_opa_t y_opa = (y_array[p_act] * LV_OPA_50) / 100;

    dsc->rect_dsc->bg_color = lv_color_mix(lv_palette_main(LV_PALETTE_RED),
                                            lv_palette_main(LV_PALETTE_BLUE),
                                            x_opa + y_opa);
  }
}

static void clk_timer_cb(lv_timer_t * timer)
{
  LV_UNUSED(timer);
  lv_obj_t * meter = (lv_obj_t *) timer->user_data;
  if(getLocalTime(local_time))
  {
    second = (int32_t) local_time->tm_sec;
    minute = (int32_t) local_time->tm_min;
    hour = (int32_t) (local_time->tm_hour%12)*5 + local_time->tm_min/12;
    lv_meter_set_indicator_end_value(meter, indic[0], second);
    lv_meter_set_indicator_end_value(meter, indic[1], minute);
    lv_meter_set_indicator_end_value(meter, indic[2], hour);
  }
}

static void add_data_1(lv_timer_t * timer)
{
  const float rad_deg = 180.0f / 3.14159265358979323846f; // Conversion factor from radians to degrees
  float acc[3], gyro[3];
  unsigned int current_time = 0;
  QMI8658_read_xyz(acc, gyro, &current_time);
  float dt = (current_time - last_time) / 1000.0f; // Convert to seconds
  if (last_time == 0) {
    dt = 0.0f; // No time difference on first run
  }
  last_time = current_time;

  // Integrate acceleration to calculate velocity and distance
  for (int i = 0; i < 3; i++) {
    velocity[i] += acc[i] * dt;          // v = v + a * dt
    distance[i] += velocity[i] * dt;    // d = d + v * dt
    angle[i] += gyro[i] * dt;    // alpha = alpha + omega * dt
  }

  static int cnt = 0;
  cnt++;
  if (cnt == 50) {
    cnt = 0;
    // Print results
    Serial.printf("Time: %u ms\n", current_time);
    Serial.printf("Acceler: x=%.2f m/s²,    y=%.2f m/s²,   z=%.2f m/s²", acc[0], acc[1], acc[2]);
    Serial.printf("    Temp: %.2f\n", QMI8658_readTemp());
    Serial.printf("Velocity: x=%.2f m/s,    y=%.2f m/s,    z=%.2f m/s\n", velocity[0], velocity[1], velocity[2]);
    Serial.printf("Distance: x=%.2f m,      y=%.2f m,      z=%.2f m\n", distance[0], distance[1], distance[2]);
    Serial.printf("Ang.Vel: da=%.2f rad/s, db=%.2f rad/s, dc=%.2f rad/s\n", gyro[0], gyro[1], gyro[2]);
    Serial.printf("Angle:    a=%.2f°,       b=%.2f°,       c=%.2f°\n", angle[0]*rad_deg, angle[1]*rad_deg, angle[2]*rad_deg);
  }

  LV_UNUSED(timer);
  lv_obj_t * chart = (lv_obj_t *) timer->user_data;
  // lv_chart_set_next_value2(chart, lv_chart_get_series_next(chart, NULL), (int) (acc[1]*scale_acc), (int) (acc[0]*scale_acc));
  lv_chart_set_next_value2(chart, lv_chart_get_series_next(chart, NULL), (distance[1]*scale_acc), (int) (distance[0]*scale_acc));
}

static void add_data_2(lv_timer_t * timer)
{
  const float rad_deg = 180.0f / 3.14159265358979323846f; // Conversion factor from radians to degrees
  float acc[3], gyro[3];
  unsigned int current_time = 0;
  QMI8658_read_xyz(acc, gyro, &current_time);
  float dt = (current_time - last_time) / 1000.0f; // Convert to seconds
  if (last_time == 0) {
    dt = 0.0f; // No time difference on first run
  }
  last_time = current_time;

  // Integrate acceleration to calculate velocity and distance
  for (int i = 0; i < 3; i++) {
    velocity[i] += acc[i] * dt;          // v = v + a * dt
    distance[i] += velocity[i] * dt;    // d = d + v * dt
    angle[i] += gyro[i] * dt;    // alpha = alpha + omega * dt
  }

  LV_UNUSED(timer);
  lv_obj_t * chart = (lv_obj_t *) timer->user_data;
  lv_chart_set_next_value2(chart, lv_chart_get_series_next(chart, NULL), (gyro[0]*scale_acc/10), (int) (-gyro[1]*scale_acc/10));
}

static void add_data_3(lv_timer_t * timer)
{
  const float rad_deg = 180.0f / 3.14159265358979323846f; // Conversion factor from radians to degrees
  float acc[3], gyro[3];
  unsigned int current_time = 0;
  QMI8658_read_xyz(acc, gyro, &current_time);
  float dt = (current_time - last_time) / 1000.0f; // Convert to seconds
  if (last_time == 0) {
    dt = 0.0f; // No time difference on first run
  }
  last_time = current_time;

  // Integrate acceleration to calculate velocity and distance
  for (int i = 0; i < 3; i++) {
    velocity[i] += acc[i] * dt;          // v = v + a * dt
    distance[i] += velocity[i] * dt;    // d = d + v * dt
    angle[i] += gyro[i] * dt;    // alpha = alpha + omega * dt
  }

  LV_UNUSED(timer);
  lv_obj_t * chart = (lv_obj_t *) timer->user_data;
  lv_chart_set_next_value2(chart, lv_chart_get_series_next(chart, NULL), (angle[0]*rad_deg), (int) (-angle[1]*rad_deg));
}

void create_clk_scr()
{
  clk_scr = lv_obj_create(NULL);
  lv_obj_add_event_cb(clk_scr, stop_clk_cb, LV_EVENT_GESTURE, NULL);

  lv_obj_t *meter = lv_meter_create(clk_scr);
  lv_obj_set_size(meter, 240, 240);
  lv_obj_set_style_bg_img_src(meter, &rolex, LV_PART_MAIN);
  lv_obj_center(meter);
  lv_obj_add_event_cb(meter, stop_clk_cb, LV_EVENT_CLICKED, NULL);

  lv_meter_scale_t * scale_min = lv_meter_add_scale(meter);
  lv_meter_set_scale_range(meter, scale_min, 0, 60, 360, 270);

  indic[0] = lv_meter_add_needle_img(meter, scale_min, &seczeiger, 38, 8);
  indic[1] = lv_meter_add_needle_img(meter, scale_min, &minzeiger, 9, 8);
  indic[2] = lv_meter_add_needle_img(meter, scale_min, &stdzeiger, 9, 15); 

  clk_timer = lv_timer_create(clk_timer_cb, 200, meter);
  lv_timer_set_repeat_count(clk_timer, -1);
  lv_timer_ready(clk_timer);
}

void create_gyr_scr(int cb_fn_nr)
{
  velocity[0] = 0.0; // Velocity in x, y, z
  velocity[1] = 0.0; // Velocity in x, y, z
  velocity[2] = 0.0; // Velocity in x, y, z
  distance[0] = 0.0; // Distance in x, y, z
  distance[1] = 0.0; // Distance in x, y, z
  distance[2] = 0.0; // Distance in x, y, z
  angle[0] = 0.0; // Distance in x, y, z
  angle[1] = 0.0; // Distance in x, y, z
  angle[2] = 0.0; // Distance in x, y, z
  last_time = 0;

  gyr_scr = lv_obj_create(NULL);
  lv_obj_add_event_cb(gyr_scr, stop_gyr_cb, LV_EVENT_GESTURE, NULL);
  lv_obj_set_style_bg_color(gyr_scr, lv_color_make(15, 50, 15), LV_PART_MAIN);

  lv_obj_t * chart = lv_chart_create(gyr_scr);
  lv_obj_add_event_cb(chart, stop_gyr_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_set_size(chart, 200, 200);
  lv_obj_align(chart, LV_ALIGN_CENTER, 0, 0);
  lv_obj_add_event_cb(chart, draw_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);
  lv_obj_set_style_line_width(chart, 0, LV_PART_ITEMS);   /*Remove the lines*/

  lv_chart_set_type(chart, LV_CHART_TYPE_SCATTER);

  lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_X, 4, 3, 5, 5, false, 20);
  lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_Y, 4, 3, 5, 5, false, 20);

  lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_X, -100, 100);
  lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, -100, 100);

  lv_chart_set_point_count(chart, 50);

  lv_chart_series_t * ser = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
  // float acc[3], gyro[3];
  // unsigned int tim_count = 0;
  // QMI8658_read_xyz(acc, gyro, &tim_count);
  // lv_chart_set_next_value2(chart, ser, (int) distance[1], (int) distance[0]);

  if (cb_fn_nr == 1) {
    gyr_timer = lv_timer_create(add_data_1, 10, chart);
  } else if (cb_fn_nr == 2) {
    gyr_timer = lv_timer_create(add_data_2, 10, chart);
  } else if (cb_fn_nr == 3) {
    gyr_timer = lv_timer_create(add_data_3, 10, chart);
  } else {
    Serial.println("create_gyr_scr: Invalid callback function number");
    return;
  }
  lv_timer_set_repeat_count(gyr_timer, -1);
}

static void weather_event_cb(lv_event_t * e)
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * chart = lv_event_get_target(e);
  int chartID = lv_obj_get_child_id(chart);

  if(code == LV_EVENT_VALUE_CHANGED) {
    lv_obj_invalidate(chart);
  }
  if(code == LV_EVENT_REFR_EXT_DRAW_SIZE) {
    lv_coord_t * s = (lv_coord_t *) lv_event_get_param(e);
    *s = LV_MAX(*s, 20);
  }
  else if(code == LV_EVENT_DRAW_POST_END) {
    int32_t id = lv_chart_get_pressed_point(chart);
    if(id == LV_CHART_POINT_NONE) return;

    LV_LOG_USER("Selected point %d", (int)id);
    Serial.printf("Selected chart / point: %d / %d\n", chartID, (int)id);

    lv_chart_series_t * ser = lv_chart_get_series_next(chart, NULL);
    boolean first_time = true;
    while(ser) {
      lv_point_t p;
      lv_chart_get_point_pos_by_id(chart, ser, id, &p);

      lv_coord_t * y_array = lv_chart_get_y_array(chart, ser);
      lv_coord_t value = y_array[id];

      char buf[16];
      int xadd = 0;
      switch (chartID)
      {
      case 0:
        {
        if (first_time) {
          lv_snprintf(buf, sizeof(buf), LV_SYMBOL_DUMMY"%.1fmm", weather_status->get_precipitation()->get_val_at_hour(id));
          first_time = false;
        }
        else
          lv_snprintf(buf, sizeof(buf), LV_SYMBOL_DUMMY"%.1f°C", weather_status->get_temperature()->get_val_at_hour(id));
        }
        break;
     case 1:
        {
        if (first_time) {
          lv_snprintf(buf, sizeof(buf), LV_SYMBOL_DUMMY"%.0f%%", weather_status->get_relative_humidity()->get_val_at_hour(id));
          first_time = false;
        }
        else
          lv_snprintf(buf, sizeof(buf), LV_SYMBOL_DUMMY"%.0f%%", weather_status->get_cloudiness()->get_val_at_hour(id));
        }
        break;
     case 2:
        {
        if (first_time) {
          lv_snprintf(buf, sizeof(buf), LV_SYMBOL_DUMMY"%.1f°C", weather_status->get_dew_point()->get_val_at_hour(id));
          first_time = false;
        }
        else {
          xadd = 5;
          lv_snprintf(buf, sizeof(buf), LV_SYMBOL_DUMMY"%.0fhPa", weather_status->get_air_pressure()->get_val_at_hour(id));
        }
        }
        break;
      case 3:
        {
        if (first_time) {
          lv_snprintf(buf, sizeof(buf), LV_SYMBOL_DUMMY"%.1f m/s", weather_status->get_wind_speeds()->get_val_at_hour(id));
          first_time = false;
        }
        else {
          lv_snprintf(buf, sizeof(buf), LV_SYMBOL_DUMMY"%.1f°", weather_status->get_wind_direction()->get_val_at_hour(id));
        }
        }
        break;
      default:
        break;
      }

      lv_draw_rect_dsc_t draw_rect_dsc;
      lv_draw_rect_dsc_init(&draw_rect_dsc);
      draw_rect_dsc.bg_color = lv_color_black();
      draw_rect_dsc.bg_opa = LV_OPA_50;
      draw_rect_dsc.radius = 3;
      draw_rect_dsc.bg_img_src = buf;
      draw_rect_dsc.bg_img_recolor = lv_color_white();

      lv_area_t a;
      a.x1 = chart->coords.x1 + p.x - (25 + xadd);
      a.x2 = chart->coords.x1 + p.x + (25 + xadd);
      a.y1 = chart->coords.y1 + p.y - 30;
      a.y2 = chart->coords.y1 + p.y - 10;

      lv_draw_ctx_t * draw_ctx = lv_event_get_draw_ctx(e);
      lv_draw_rect(draw_ctx, &draw_rect_dsc, &a);

      ser = lv_chart_get_series_next(chart, ser);
    }
  }
  else if(code == LV_EVENT_RELEASED) {
      lv_obj_invalidate(chart);
  }
}

void create_weather_scr()
{
  weather_scr = lv_obj_create(NULL);
  lv_obj_add_event_cb(weather_scr, stop_weather_cb, LV_EVENT_GESTURE, NULL);
  lv_obj_set_style_bg_color(weather_scr, lv_color_make(15, 50, 15), LV_PART_MAIN);

  lv_obj_t *cont = lv_obj_create(weather_scr);
  lv_obj_set_size(cont, 240, 240);
  lv_obj_center(cont);
  lv_obj_set_style_bg_color(cont, lv_color_make(0,0,35), LV_PART_MAIN);
  lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
  // lv_obj_add_event_cb(cont, scroll_event_cb, LV_EVENT_SCROLL, NULL);
  // lv_obj_set_style_radius(cont, LV_RADIUS_CIRCLE, 0);
  // lv_obj_set_style_clip_corner(cont, true, 0);
  lv_obj_set_scroll_dir(cont, LV_DIR_VER);
  lv_obj_set_scroll_snap_y(cont, LV_SCROLL_SNAP_CENTER);
  lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);

  lv_obj_t *chart, *label;
  lv_coord_t yminp, ymaxp, ymins, ymaxs;
  lv_chart_series_t *ser1, *ser2;

  // Temperature and precipation
  yminp = (lv_coord_t) (0.f*weather_status->get_temperature()->get_factor());
  ymaxp = (lv_coord_t) (30.f*weather_status->get_temperature()->get_factor());
  ymins = (lv_coord_t) (0.f*weather_status->get_precipitation()->get_factor());
  ymaxs = (lv_coord_t) (5.f*weather_status->get_precipitation()->get_factor());
  chart = lv_chart_create(cont);
  lv_obj_add_event_cb(chart, stop_weather_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_set_size(chart, 220, 220);
  lv_obj_center(chart);
  lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_X, 0, weather_status->get_num_hours());
  lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, yminp, ymaxp);
  lv_chart_set_range(chart, LV_CHART_AXIS_SECONDARY_Y, ymins, ymaxs);
  lv_obj_add_event_cb(chart, weather_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_refresh_ext_draw_size(chart);
  lv_chart_set_zoom_x(chart, 256);
  lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_X, 4, 3, 5, 5, true, 20);
  lv_chart_set_point_count(chart, weather_status->get_num_hours()+1);
  ser1 = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
  ser2 = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_SECONDARY_Y);
  for(int i = 0; i <= weather_status->get_num_hours(); i++) {
      lv_chart_set_next_value(chart, ser1, weather_status->get_temperature()->get_val_at_hour_raw(i));
      lv_chart_set_next_value(chart, ser2, weather_status->get_precipitation()->get_val_at_hour_raw(i));
      Serial.printf("#/t/pr/cl/p/v/RF/dir/DP %d %.1f  %.1f  %.1f  %.1f  %.1f  %.1f  %.1f  %.1f\n",i,
              weather_status->get_temperature()->get_val_at_hour(i),
              weather_status->get_precipitation()->get_val_at_hour(i),
              weather_status->get_cloudiness()->get_val_at_hour(i),
              weather_status->get_air_pressure()->get_val_at_hour(i),
              weather_status->get_wind_speeds()->get_val_at_hour(i),
              weather_status->get_relative_humidity()->get_val_at_hour(i),
              weather_status->get_wind_direction()->get_val_at_hour(i),
              weather_status->get_dew_point()->get_val_at_hour(i));
  }
  label = lv_label_create(chart);
  lv_label_set_recolor(label, true);                      /*Enable re-coloring by commands in the text*/
  lv_label_set_text_fmt(label, "#ff0000 temperature#\n#0000ff precipation#");
  lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_LEFT, 0);
  lv_obj_align(label, LV_ALIGN_TOP_LEFT, 10, 10);


  // Cloudiness and humidity
  yminp = (lv_coord_t) (0.f*weather_status->get_cloudiness()->get_factor());
  ymaxp = (lv_coord_t) (100.f*weather_status->get_cloudiness()->get_factor());
  ymins = (lv_coord_t) (0.f*weather_status->get_relative_humidity()->get_factor());
  ymaxs = (lv_coord_t) (100.f*weather_status->get_relative_humidity()->get_factor());
  chart = lv_chart_create(cont);
  lv_obj_add_event_cb(chart, stop_weather_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_set_size(chart, 220, 220);
  lv_obj_center(chart);
  lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_X, 0, weather_status->get_num_hours());
  lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, yminp, ymaxp);
  lv_chart_set_range(chart, LV_CHART_AXIS_SECONDARY_Y, ymins, ymaxs);
  lv_obj_add_event_cb(chart, weather_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_refresh_ext_draw_size(chart);
  lv_chart_set_zoom_x(chart, 256);
  lv_chart_set_point_count(chart, weather_status->get_num_hours()+1);
  ser1 = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_GREY), LV_CHART_AXIS_PRIMARY_Y);
  ser2 = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_PURPLE), LV_CHART_AXIS_SECONDARY_Y);
  for(int i = 0; i <= weather_status->get_num_hours(); i++) {
      lv_chart_set_next_value(chart, ser1, weather_status->get_cloudiness()->get_val_at_hour_raw(i));
      lv_chart_set_next_value(chart, ser2, weather_status->get_relative_humidity()->get_val_at_hour_raw(i));
  }
  label = lv_label_create(chart);
  lv_label_set_recolor(label, true);                      /*Enable re-coloring by commands in the text*/
  lv_label_set_text_fmt(label, "#666666 cloudiness#\n#711a83 humidity#");
  lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_LEFT, 0);
  lv_obj_align(label, LV_ALIGN_TOP_LEFT, 10, 10);

  // Air pressure and dew point
  yminp = (lv_coord_t) (980.f*weather_status->get_air_pressure()->get_factor());
  ymaxp = (lv_coord_t) (1030.f*weather_status->get_air_pressure()->get_factor());
  ymins = (lv_coord_t) (0.f*weather_status->get_dew_point()->get_factor());
  ymaxs = (lv_coord_t) (30.f*weather_status->get_dew_point()->get_factor());
  chart = lv_chart_create(cont);
  lv_obj_add_event_cb(chart, stop_weather_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_set_size(chart, 220, 220);
  lv_obj_center(chart);
  lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_X, 0, weather_status->get_num_hours());
  lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, yminp, ymaxp);
  lv_chart_set_range(chart, LV_CHART_AXIS_SECONDARY_Y, ymins, ymaxs);
  lv_obj_add_event_cb(chart, weather_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_refresh_ext_draw_size(chart);
  lv_chart_set_zoom_x(chart, 256);
  lv_chart_set_point_count(chart, weather_status->get_num_hours()+1);
  ser1 = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_PRIMARY_Y);
  ser2 = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_BROWN), LV_CHART_AXIS_SECONDARY_Y);
  for(int i = 0; i <= weather_status->get_num_hours(); i++) {
      lv_chart_set_next_value(chart, ser1, weather_status->get_air_pressure()->get_val_at_hour_raw(i));
      lv_chart_set_next_value(chart, ser2, weather_status->get_dew_point()->get_val_at_hour_raw(i));
  }
  label = lv_label_create(chart);
  lv_label_set_recolor(label, true);                      /*Enable re-coloring by commands in the text*/
  lv_label_set_text_fmt(label, "#0000ff air pressure#\n#2b1a08 dew point#");
  lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_LEFT, 0);
  lv_obj_align(label, LV_ALIGN_TOP_LEFT, 10, 10);

  // Wind direction & speed
  yminp = (lv_coord_t) (0.f*weather_status->get_wind_direction()->get_factor());
  ymaxp = (lv_coord_t) (360.f*weather_status->get_wind_direction()->get_factor());
  ymins = (lv_coord_t) (0.f*weather_status->get_wind_speeds()->get_factor());
  ymaxs = (lv_coord_t) (10.f*weather_status->get_wind_speeds()->get_factor());
  chart = lv_chart_create(cont);
  lv_obj_add_event_cb(chart, stop_weather_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_set_size(chart, 220, 220);
  lv_obj_center(chart);
  lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_X, 0, weather_status->get_num_hours());
  lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, yminp, ymaxp);
  lv_chart_set_range(chart, LV_CHART_AXIS_SECONDARY_Y, ymins, ymaxs);
  lv_obj_add_event_cb(chart, weather_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_refresh_ext_draw_size(chart);
  lv_chart_set_zoom_x(chart, 256);
  lv_chart_set_point_count(chart, weather_status->get_num_hours()+1);
  ser1 = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_ORANGE), LV_CHART_AXIS_PRIMARY_Y);
  ser2 = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_GREEN), LV_CHART_AXIS_SECONDARY_Y);
  for(int i = 0; i <= weather_status->get_num_hours(); i++) {
      lv_chart_set_next_value(chart, ser1, weather_status->get_wind_direction()->get_val_at_hour_raw(i));
      lv_chart_set_next_value(chart, ser2, weather_status->get_wind_speeds()->get_val_at_hour_raw(i));
  }
  label = lv_label_create(chart);
  lv_label_set_recolor(label, true);                      /*Enable re-coloring by commands in the text*/
  lv_label_set_text_fmt(label, "#ff8c00 wind direction#\n#00ff00 wind speed#");
  lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_LEFT, 0);
  lv_obj_align(label, LV_ALIGN_TOP_LEFT, 10, 10);
}

static void btn_std_event_cb(lv_event_t * e)
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * btn = (lv_obj_t *) lv_event_get_target(e);
  if(code == LV_EVENT_CLICKED && !screenIsOn()) {
    backlightOn();
  }
  else if(code == LV_EVENT_CLICKED) {
    backlightOn();
    uint32_t idx = lv_obj_get_index(btn);
    /*Get the first child of the button which is the label and change its text*/
    lv_obj_t * label = lv_obj_get_child(btn, 0);
    Serial.print("Clicked on ");
    Serial.println(lv_label_get_text(label));
    switch(idx) {
    case 4:
      {
      if(getLocalTime(local_time))
      {
        lv_label_set_text_fmt(label, "Time: %d:%02d:%02d", local_time->tm_hour, local_time->tm_min, local_time->tm_sec);
      }
      create_clk_scr();
      lv_scr_load(clk_scr);
      }
      break;
    case 5:
      {
      create_weather_scr();
      lv_scr_load(weather_scr);
      }
      break;
    case 6:
      {
      create_gyr_scr(1);
      lv_scr_load(gyr_scr);
      }
      break;
    case 7:
      {
      create_gyr_scr(2);
      lv_scr_load(gyr_scr);
      }
      break;
    case 8:
      {
      create_gyr_scr(3);
      lv_scr_load(gyr_scr);
      }
      break;
    case 9:
      {
      static uint8_t cnt = 0;
      cnt++;
      lv_label_set_text_fmt(label, "Alex: %d", cnt);
      img_scr = lv_obj_create(NULL);
      lv_obj_add_event_cb(img_scr, stop_img_cb, LV_EVENT_GESTURE, NULL);
      lv_obj_t * img1 = lv_img_create(img_scr);
      lv_img_set_src(img1, &image2); // Pfad zum JPG-Bild im SPIFFS
      lv_img_set_pivot(img1, 120, 120);    /*Rotate around (100, 100)*/
      lv_anim_t a;
      lv_anim_init(&a);
      lv_anim_set_var(&a, img1);
      lv_anim_set_exec_cb(&a, set_angle);
      lv_anim_set_values(&a, 0, 3600);
      lv_anim_set_time(&a, 3000);
      lv_anim_set_repeat_count(&a, 3);
      lv_anim_start(&a);
  
      lv_anim_set_exec_cb(&a, set_scale);
      lv_anim_set_values(&a, 16, 256);
      lv_anim_set_playback_time(&a, 3000);
      lv_anim_start(&a);
      lv_scr_load(img_scr);
      }
      break;
    case 10:
      {
      static uint8_t cnt = 0;
      cnt++;
      lv_label_set_text_fmt(label, "Bums: %d", cnt);
      img_scr = lv_obj_create(NULL);
      lv_obj_add_event_cb(img_scr, stop_img_cb, LV_EVENT_GESTURE, NULL);
      lv_obj_t * img1 = lv_img_create(img_scr);
      lv_img_set_src(img1, &image1); // Pfad zum JPG-Bild im SPIFFS
      lv_img_set_pivot(img1, 100, 100);    /*Rotate around (100, 100)*/
      lv_anim_t a;
      lv_anim_init(&a);
      lv_anim_set_var(&a, img1);
      lv_anim_set_exec_cb(&a, set_angle);
      lv_anim_set_values(&a, 0, 3600);
      lv_anim_set_time(&a, 5000);
      lv_anim_set_repeat_count(&a, 5);
      lv_anim_start(&a);
  
      lv_anim_set_exec_cb(&a, set_scale);
      lv_anim_set_values(&a, 128, 256);
      lv_anim_set_playback_time(&a, 3000);
      lv_anim_start(&a);
      lv_scr_load(img_scr);
      }
      break;
    default:
      {
      Serial.println("Still to be implemented");
      }
      break;
    }
  }
}

static void set_angle(void * img, int32_t v)
{
  lv_img_set_angle((lv_obj_t *)img, v);
}

static void set_scale(void * img, int32_t v)
{
  lv_img_set_zoom((lv_obj_t *)img, v);
}

static void saveWeatherData()
{
  for (int i = 0; i < weather_status->get_num_hours()+1; i++)
  {
    temperature[i] = weather_status->get_temperature()->get_val_at_hour_raw(i);
    precipitation[i] = weather_status->get_precipitation()->get_val_at_hour_raw(i);
    cloudiness[i] = weather_status->get_cloudiness()->get_val_at_hour_raw(i);
    pressure[i] = weather_status->get_air_pressure()->get_val_at_hour_raw(i);
    wind_speed[i] = weather_status->get_wind_speeds()->get_val_at_hour_raw(i);
    humidity[i] = weather_status->get_relative_humidity()->get_val_at_hour_raw(i);
    wind_direction[i] = weather_status->get_wind_direction()->get_val_at_hour_raw(i);
    dew_point[i] = weather_status->get_dew_point()->get_val_at_hour_raw(i);
  }
  weather_1h = weather_status->get_symbol_code_next_1h();
  weather_6h = weather_status->get_symbol_code_next_6h();
  weather_12h = weather_status->get_symbol_code_next_12h();
  expiredTime.tm_hour = weather_status->getExpiredTime()->tm_hour;
  expiredTime.tm_min = weather_status->getExpiredTime()->tm_min;
  expiredTime.tm_sec = weather_status->getExpiredTime()->tm_sec;
  expiredTime.tm_year = weather_status->getExpiredTime()->tm_year;
  expiredTime.tm_mon = weather_status->getExpiredTime()->tm_mon;
  expiredTime.tm_mday = weather_status->getExpiredTime()->tm_mday;
}

void deep_sleep_timer_cb(lv_timer_t * timer)
{
  if (deepSleep)
  {
    Serial.println("deep_sleep_timer_cb: going to sleep!");
    getLocalTime(&timeinfo); // Aktuelle Zeit abrufen und in timeinfo speichern
    saveWeatherData(); // Wetterdaten speichern
    Serial.printf("Uhrzeit beim Einschlafen: %2d:%2d:%2d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    esp_deep_sleep_start();
  }
}

void screen_on_timer_cb(lv_timer_t * timer)
{
//    Serial.println("screen_on_timer_cb: turning off!");
  backlightOff();
}

void bat_timer_cb(lv_timer_t * timer)
{
  lv_obj_t * label = (lv_obj_t *) timer->user_data;
  static lv_anim_t animation_template;
  static lv_style_t label_style;

  lv_anim_init(&animation_template);
  lv_anim_set_delay(&animation_template, 1000);           /*Wait 1 second to start the first scroll*/
  lv_anim_set_repeat_delay(&animation_template, 3000);    /*Repeat the scroll 3 seconds after the label scrolls back to the initial position*/

  /*Initialize the label style with the animation template*/
  lv_style_init(&label_style);
  lv_style_set_anim(&label_style, &animation_template);

  lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);      /*Circular scroll*/
  lv_obj_set_width(label, 130);
  float bat_voltage = my_DEC_ADC_Read()*conversion_factor;
  int bat_percentage = map((int) 1000*bat_voltage, 3100, 4200, 0, 100);
  if (bat_voltage < 4.3)
  {
    if (bat_percentage > 80)
      lv_label_set_text_fmt( label, LV_SYMBOL_BATTERY_FULL" Akkustand: %4.3f V oder %d %%", bat_voltage, bat_percentage);
    else if (bat_percentage > 60)
      lv_label_set_text_fmt( label, LV_SYMBOL_BATTERY_3" Akkustand: %4.3f V oder %d %%", bat_voltage, bat_percentage);
    else if (bat_percentage > 40)
      lv_label_set_text_fmt( label, LV_SYMBOL_BATTERY_2" Akkustand: %4.3f V oder %d %%", bat_voltage, bat_percentage);
    else if (bat_percentage > 20)
      lv_label_set_text_fmt( label, LV_SYMBOL_BATTERY_1" Akkustand: %4.3f V oder %d %%", bat_voltage, bat_percentage);
    else
      lv_label_set_text_fmt( label, LV_SYMBOL_BATTERY_EMPTY" Akkustand: %4.3f V oder %d %%", bat_voltage, bat_percentage);
  }
  else
  {
    lv_label_set_text(label, LV_SYMBOL_CHARGE" Akku wird am USB C geladen.");
  }
  lv_obj_add_style(label, &label_style, LV_STATE_DEFAULT);           /*Add the style to the label*/
}
 
static void scroll_event_cb(lv_event_t * e)
{
  lv_obj_t * cont = lv_event_get_target(e);

  lv_area_t cont_a;
  lv_obj_get_coords(cont, &cont_a);
  lv_coord_t cont_y_center = cont_a.y1 + lv_area_get_height(&cont_a) / 2;

  lv_coord_t r = lv_obj_get_height(cont) * 7 / 10;
  uint32_t i;
  uint32_t child_cnt = lv_obj_get_child_cnt(cont);
  for(i = 0; i < child_cnt; i++) {
    lv_obj_t * child = lv_obj_get_child(cont, i);
    lv_area_t child_a;
    lv_obj_get_coords(child, &child_a);

    lv_coord_t child_y_center = child_a.y1 + lv_area_get_height(&child_a) / 2;

    lv_coord_t diff_y = child_y_center - cont_y_center;
    diff_y = LV_ABS(diff_y);

    /*Get the x of diff_y on a circle.*/
    lv_coord_t x;
    /*If diff_y is out of the circle use the last point of the circle (the radius)*/
    if(diff_y >= r) {
      x = r;
    }
    else {
      /*Use Pythagoras theorem to get x from radius and y*/
      uint32_t x_sqr = r * r - diff_y * diff_y;
      lv_sqrt_res_t res;
      lv_sqrt(x_sqr, &res, 0x8000);   /*Use lvgl's built in sqrt root function*/
      x = r - res.i;
    }

    /*Translate the item by the calculated X coordinate*/
    lv_obj_set_style_translate_x(child, x, 0);

    /*Use some opacity with larger translations*/
    // lv_opa_t opa = lv_map(x, 0, r, LV_OPA_TRANSP, LV_OPA_COVER);
    // lv_obj_set_style_opa(child, LV_OPA_COVER - opa, 0);
    backlightOn();
  }
}

void draw_std_scr()
{
  // Initialisiere LVGL
  // lv_init();
  // Serial.print("init ... ");
  // lv_tick_set_cb(my_tick_get_cb);
  // Serial.print("tick ... ");
  // lv_display_t * display1 = lv_display_create(LCD_1IN28_WIDTH, LCD_1IN28_HEIGHT);
  // Serial.print("display ... ");
  // lv_display_set_buffers(display1, BlackImage, NULL, 240*240*2, LV_DISPLAY_RENDER_MODE_DIRECT);
  // Serial.print("buffer ... ");
  // lv_display_set_flush_cb(display1, my_flush_cb);
  // Serial.print("flush ... ");
  // lv_indev_t * indev = lv_indev_create();        /* Create input device connected to Default Display. */
  // Serial.print("indev ... ");
  // lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);   /* Touch pad is a pointer-like device. */
  // lv_indev_set_read_cb(indev, my_input_read);    /* Set driver function. */
  // Serial.print("read ... ");
  // register a fs and define the driver
  // static lv_fs_drv_t drv;                   /* Needs to be static or global */
  // lv_fs_drv_init(&drv);                     /* Basic initialization */
  // drv.letter = 'Z';                         /* An uppercase letter to identify the drive */
  // drv.cache_size = 0;                       /* Cache size for reading in bytes. 0 to not cache. */
  // drv.open_cb = fs_open;                 /* Callback to open a file */
  // drv.close_cb = fs_close;               /* Callback to close a file */
  // drv.read_cb = fs_read;                 /* Callback to read a file */
  // drv.seek_cb = fs_seek;                 /* Callback to seek in a file (Move cursor) */
  // drv.tell_cb = fs_tell;                 /* Callback to tell the cursor position  */
  // lv_fs_drv_register(&drv);                 /* Finally register the drive */

  std_scr = lv_obj_create(NULL);
  lv_obj_add_event_cb(std_scr, screen_wake_up_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_set_style_bg_color(std_scr, lv_color_hex(0x800080), LV_PART_MAIN);

  lv_obj_t *cont = lv_obj_create(std_scr);
  lv_obj_set_size(cont, 200, 200);
  lv_obj_center(cont);
  lv_obj_set_style_bg_color(cont, lv_color_make(0,0,35), LV_PART_MAIN);
  lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
  lv_obj_add_event_cb(cont, scroll_event_cb, LV_EVENT_SCROLL, NULL);
  lv_obj_set_style_radius(cont, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_clip_corner(cont, true, 0);
  lv_obj_set_scroll_dir(cont, LV_DIR_VER);
  lv_obj_set_scroll_snap_y(cont, LV_SCROLL_SNAP_CENTER);
  lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);

  lv_obj_t *wifi_label = lv_label_create(cont); // Label erstellen
  lv_label_set_text(wifi_label, LV_SYMBOL_WIFI LV_SYMBOL_CLOSE);
  lv_obj_set_style_text_color(wifi_label, lv_color_make(255, 255, 255), LV_PART_MAIN);
  lv_obj_set_style_bg_color(wifi_label, lv_obj_get_style_bg_color(cont, LV_PART_MAIN), LV_PART_MAIN);
  lv_obj_set_style_radius(wifi_label, lv_obj_get_style_radius(cont, LV_PART_MAIN), LV_PART_MAIN);
  lv_obj_set_style_clip_corner(wifi_label, lv_obj_get_style_clip_corner(cont, LV_PART_MAIN), LV_PART_MAIN);
//  lv_obj_set_style_border_width(wifi_label, 2, LV_PART_MAIN); // Rahmenbreite
//  lv_obj_set_style_border_color(wifi_label, lv_color_make(255, 255, 255), LV_PART_MAIN); // Rahmenfarbe (weiß)

  lv_obj_t *bat_label = lv_label_create(cont); // Label erstellen
  lv_label_set_text(bat_label, LV_SYMBOL_BATTERY_FULL" Status");
  lv_obj_set_style_text_color(bat_label, lv_color_make(255, 255, 255), LV_PART_MAIN);
  lv_obj_set_style_bg_color(bat_label, lv_obj_get_style_bg_color(cont, LV_PART_MAIN), LV_PART_MAIN);
  lv_obj_set_style_radius(bat_label, lv_obj_get_style_radius(cont, LV_PART_MAIN), LV_PART_MAIN);
  lv_obj_set_style_clip_corner(bat_label, lv_obj_get_style_clip_corner(cont, LV_PART_MAIN), LV_PART_MAIN);
//  lv_obj_set_style_border_width(bat_label, 2, LV_PART_MAIN); // Rahmenbreite
//  lv_obj_set_style_border_color(bat_label, lv_color_make(255, 255, 255), LV_PART_MAIN); // Rahmenfarbe (weiß)
  lv_timer_t *bat_timer = lv_timer_create(bat_timer_cb, 60000, bat_label);
  lv_timer_set_repeat_count(bat_timer, -1);
  lv_timer_ready(bat_timer);

  lv_obj_t *row_cont, *switch_label, *sw;
  row_cont = lv_obj_create(cont); // Container für Switch und Label
  lv_obj_set_flex_flow(row_cont, LV_FLEX_FLOW_ROW); // Anordnung in einer Zeile
  lv_obj_set_size(row_cont, lv_pct(100), LV_SIZE_CONTENT); // Breite 100%, Höhe automatisch
  lv_obj_set_style_pad_row(row_cont, 10, 0); // Abstand zwischen den Elementen
  lv_obj_set_style_bg_color(row_cont, lv_obj_get_style_bg_color(cont, LV_PART_MAIN), LV_PART_MAIN);
  lv_obj_set_style_radius(row_cont, lv_obj_get_style_radius(cont, LV_PART_MAIN), LV_PART_MAIN);
  lv_obj_set_style_clip_corner(row_cont, lv_obj_get_style_clip_corner(cont, LV_PART_MAIN), LV_PART_MAIN);
  lv_obj_set_style_border_color(row_cont, lv_obj_get_style_bg_color(cont, LV_PART_MAIN), LV_PART_MAIN); // Rahmenfarbe (weiß)

  switch_label = lv_label_create(row_cont); // Label erstellen
  lv_label_set_text(switch_label, LV_SYMBOL_WIFI" ein/aus");
  lv_obj_set_style_text_color(switch_label, lv_color_make(255, 255, 255), LV_PART_MAIN);
  
  sw = lv_switch_create(row_cont); // Switch erstellen
  lv_obj_add_event_cb(sw, [](lv_event_t * e) {
      lv_obj_t *sw = lv_event_get_target(e);
      if (lv_obj_has_state(sw, LV_STATE_CHECKED))
      {
        // WiFi aktivieren
        if (WiFi.status() != WL_CONNECTED)
        {
          Serial.print("Connecting to ");
          Serial.println(ssid);
          WiFi.mode(WIFI_STA);
          if (changeMAC)
          {
            Serial.print("OLD ESP32 MAC Address: ");
            Serial.println(WiFi.macAddress());
            esp_wifi_set_mac(WIFI_IF_STA, newMACAddress);  // Change MAC-address inc case of captive portals
            Serial.print("NEW ESP32 MAC Address: ");
            Serial.println(WiFi.macAddress());
          }
          WiFi.begin(ssid, password);
//           WiFi.begin(ssid);
          lv_obj_t * label = lv_obj_get_child(lv_obj_get_child(std_scr, 0), 0);
          lv_label_set_text(label, LV_SYMBOL_WIFI LV_SYMBOL_WARNING);
        }
        Serial.println("Wifi switched ON");
      }
      else
      {
        // WiFi deaktivieren
        WiFi.disconnect();
        WiFi.mode(WIFI_OFF);
        lv_obj_t * label = lv_obj_get_child(lv_obj_get_child(std_scr, 0), 0);
        lv_label_set_text(label, LV_SYMBOL_WIFI LV_SYMBOL_CLOSE);
        Serial.println("Wifi switched OFF");
      }
  }, LV_EVENT_VALUE_CHANGED, NULL);
  lv_obj_set_size(sw, 50, 18); // Größe des Switches anpassen
  lv_obj_set_style_bg_color(sw, lv_color_make(0, 0, 100), LV_PART_MAIN);
  lv_obj_set_style_bg_color(sw, lv_color_make(0, 0, 0), LV_PART_KNOB);

  row_cont = lv_obj_create(cont); // Container für Switch und Label
  lv_obj_set_flex_flow(row_cont, LV_FLEX_FLOW_ROW); // Anordnung in einer Zeile
  lv_obj_set_size(row_cont, lv_pct(100), LV_SIZE_CONTENT); // Breite 100%, Höhe automatisch
  lv_obj_set_style_pad_row(row_cont, 10, 0); // Abstand zwischen den Elementen
  lv_obj_set_style_bg_color(row_cont, lv_obj_get_style_bg_color(cont, LV_PART_MAIN), LV_PART_MAIN);
  lv_obj_set_style_radius(row_cont, lv_obj_get_style_radius(cont, LV_PART_MAIN), LV_PART_MAIN);
  lv_obj_set_style_clip_corner(row_cont, lv_obj_get_style_clip_corner(cont, LV_PART_MAIN), LV_PART_MAIN);
  lv_obj_set_style_border_color(row_cont, lv_obj_get_style_bg_color(cont, LV_PART_MAIN), LV_PART_MAIN); // Rahmenfarbe (weiß)

  switch_label = lv_label_create(row_cont); // Label erstellen
  lv_label_set_text(switch_label, LV_SYMBOL_POWER" lo pwr");
  lv_obj_set_style_text_color(switch_label, lv_color_make(255, 255, 255), LV_PART_MAIN);
  
  sw = lv_switch_create(row_cont); // Switch erstellen
  lv_obj_add_event_cb(sw, [](lv_event_t * e) {
      lv_obj_t *sw = lv_event_get_target(e);
      if (lv_obj_has_state(sw, LV_STATE_CHECKED))
      {
        // enable deep sleep
        deepSleep = true;
        Serial.println("DeepSleep enabled");
      }
      else
      {
        // disable deep sleep
        deepSleep = false;
        Serial.println("DeepSleep disabled");
      }
  }, LV_EVENT_VALUE_CHANGED, NULL);
  lv_obj_set_size(sw, 50, 18); // Größe des Switches anpassen
  lv_obj_set_style_bg_color(sw, lv_color_make(0, 0, 100), LV_PART_MAIN);
  lv_obj_set_style_bg_color(sw, lv_color_make(0, 0, 0), LV_PART_KNOB);

  uint32_t i;
  for(i = 0; i < 10; i++) {
      lv_obj_t * btn = lv_btn_create(cont);
      lv_obj_set_width(btn, lv_pct(100));
      lv_obj_add_event_cb(btn, btn_std_event_cb, LV_EVENT_ALL, NULL); /*Assign a callback to the button*/

      lv_obj_t * label = lv_label_create(btn);
      lv_label_set_text_fmt(label, "Button %"LV_PRIu32, i);
      lv_obj_set_style_text_color(label, lv_color_make(0,0,0), LV_PART_MAIN);
  }
  lv_label_set_text(lv_obj_get_child(lv_obj_get_child(cont, 4), -1), "Uhrzeit");
  lv_label_set_text(lv_obj_get_child(lv_obj_get_child(cont, 5), -1), "Wetter");
  lv_label_set_text(lv_obj_get_child(lv_obj_get_child(cont, 6), -1), "Distance");
  lv_label_set_text(lv_obj_get_child(lv_obj_get_child(cont, 7), -1), "Gyro");
  lv_label_set_text(lv_obj_get_child(lv_obj_get_child(cont, 8), -1), "Angles");
  lv_label_set_text(lv_obj_get_child(lv_obj_get_child(cont, 9), -1), "Alex go");
  lv_label_set_text(lv_obj_get_child(lv_obj_get_child(cont, 10), -1), "Bums go");

  /*Update the buttons position manually for first*/
  lv_event_send(cont, LV_EVENT_SCROLL, NULL);

  /*Be sure the fist button is in the middle*/
  lv_obj_scroll_to_view(lv_obj_get_child(cont, 0), LV_ANIM_OFF);

  lv_scr_load(std_scr);
  Serial.println("lvgl initialisiert.");
}
 

/* Display flushing */
void my_disp_flush( lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p )
{
  uint32_t w = ( area->x2 - area->x1 + 1 );
  uint32_t h = ( area->y2 - area->y1 + 1 );

  tft.startWrite();
  tft.setAddrWindow( area->x1, area->y1, w, h );
  tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
  tft.endWrite();

  lv_disp_flush_ready( disp_drv );
}

void example_increase_lvgl_tick(void *arg)
{
  /* Tell LVGL how many milliseconds has elapsed */
  lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
}

// static uint8_t count=0;
// void example_increase_reboot(void *arg)
// {
//   count++;
//   if(count==30){
//     // esp_restart();
//   }
// }

/*Read the touchpad*/
void my_touchpad_read( lv_indev_drv_t * indev_drv, lv_indev_data_t * data )
{
  // uint16_t touchX, touchY;

  bool touched = touch.available();
  // touch.read_touch();
  if( !touched )
  // if( 0!=touch.data.points )
  {
      data->state = LV_INDEV_STATE_REL;
  }
  else
  {
    data->state = LV_INDEV_STATE_PR;

    /*Set the coordinates*/
    data->point.x = touch.data.x;
    data->point.y = touch.data.y;
    // Serial.print( "Data x " );
    // Serial.println( touch.data.x );

    // Serial.print( "Data y " );
    // Serial.println( touch.data.y );
  }
}

uint16_t my_DEC_ADC_Read()
{
  return analogReadMilliVolts(BAT_ADC_PIN);
}
 
void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  Serial.println("Wifi connected to AP successfully!");
  configTzTime("WEST-1DWEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", "ptbtime1.ptb.de", "rustime01.rus.uni-stuttgart.de");
  server.on("/", []() {server.send(200, "text/plain", "Hi! This is the Webserver of the little TFT guy from Alex.");});
  ElegantOTA.begin(&server);    // Start ElegantOTA
  ElegantOTA.onStart(onOTAStart);
  ElegantOTA.onProgress(onOTAProgress);
  ElegantOTA.onEnd(onOTAEnd);
  server.begin();
  lv_obj_t * label = lv_obj_get_child(lv_obj_get_child(std_scr, 0), 0);
  lv_label_set_text(label, LV_SYMBOL_WIFI LV_SYMBOL_OK);
  // delay(5000);
  // if( weather_status->is_expired()) {
  //   weather_status->update_data();
  // }
  // delay(2000);
  // Serial.println(weather_status->getExpiredTime());
  // if( weather_status->is_expired()) {
  //   weather_status->update_data();
  // }
  // Serial.println(weather_status->getExpiredTime());
  // Serial.print("Weather data: ");
  // Serial.print("Current temperature: ");
  // Serial.print((weather_status->get_temperature())->get_current());
  // Serial.println("°C");
  // Serial.print("Maximum temperature next 24 hours: ");
  // Serial.print((weather_status->get_temperature())->get_maximum());
  // Serial.println("°C");

  // Serial.print("Minimum temperature next 24 hours: ");
  // Serial.print((weather_status->get_temperature())->get_minimum());
  // Serial.println("°C");

  // Serial.print("Mean temperature next 24 hours: ");
  // Serial.print((weather_status->get_temperature())->get_mean());
  // Serial.println("°C");
}

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
  Serial.print("WiFi connected to IP address: ");
  Serial.println(WiFi.localIP());
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  Serial.println("Disconnected from WiFi access point");
  Serial.print("WiFi lost connection. Reason: ");
  Serial.println(info.wifi_sta_disconnected.reason);
  server.stop();
  lv_obj_t * label = lv_obj_get_child(lv_obj_get_child(std_scr, 0), 0);
  lv_label_set_text(label, LV_SYMBOL_WIFI LV_SYMBOL_CLOSE);
}

void configureTimezone(const char* timezone)
{
    setenv("TZ", timezone, 1); // Setze die Zeitzone
    tzset();                   // Aktualisiere die Zeitzoneneinstellungen
    Serial.print("Timezone set to: ");
    Serial.println(timezone);
}

void setAndResetTime()
{
    // Prüfen, ob der ESP32 aus Deep Sleep aufgewacht ist
    if (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_UNDEFINED) {
      // Zeitspanne in Mikrosekunden seit Beginn Deep Sleep
      configureTimezone("WEST-1DWEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00");
      getLocalTime(&timeinfo); // Aktuelle Zeit abrufen
      Serial.printf("Uhrzeit beim Aufwachen: %2d:%2d:%2d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    }
    else
    {
      timeinfo.tm_year = 2025 - 1900; // Jahre seit 1900
      timeinfo.tm_mon = 4;           // Monate seit Januar (0-11)
      timeinfo.tm_mday = 20;         // Tag des Monats
      timeinfo.tm_hour = 12;
      timeinfo.tm_min = 30;
      timeinfo.tm_sec = 0;
      time_t t = mktime(&timeinfo);
      struct timeval now = { .tv_sec = t };
      settimeofday(&now, NULL); // Setze die Systemzeit
      configureTimezone("WEST-1DWEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00");
      Serial.printf("Manual time set to: %2d:%2d:%2d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    }
}

void restoreWeatherData()
{
  // Restore weather data
  for (int i = 0; i < weather_status->get_num_hours()+1; i++)
  {
    weather_status->get_temperature()->update_vals(temperature);
    weather_status->get_precipitation()->update_vals(precipitation);
    weather_status->get_cloudiness()->update_vals(cloudiness);
    weather_status->get_air_pressure()->update_vals(pressure);
    weather_status->get_wind_speeds()->update_vals(wind_speed);
    weather_status->get_relative_humidity()->update_vals(humidity);
    weather_status->get_wind_direction()->update_vals(wind_direction);
    weather_status->get_dew_point()->update_vals(dew_point);
  }
  weather_status->set_symbol_code_next_1h(weather_1h);
  weather_status->set_symbol_code_next_6h(weather_6h);
  weather_status->set_symbol_code_next_12h(weather_12h);
  weather_status->setExpiredTime(&expiredTime);
}

void setup()
{
    Serial.begin( 115200 ); /* prepare for possible serial debug */

    setAndResetTime();
    local_time = new tm [1];

    // Define the GPIO for wakeup: set to the interrupt pin of the touchscreen
    esp_sleep_enable_ext0_wakeup(WAKEUP_GPIO, 0);  // 1 = High, 0 = Low
    
    String LVGL_Arduino = "Hello Arduino! ";
    LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

    Serial.println( LVGL_Arduino );
    Serial.println( "I am LVGL_Arduino" );

    weather_status = new WeatherRFP( 48.799909f, 9.652092f, 360); // Weitmars, Germany
//     weather_status = new WeatherRFP( 58.967755f, 5.731111f, 8); // Stavanger, Norway
    weather_status->set_utc_offset(1);
    weather_status->set_daylight_saving(true);
    restoreWeatherData(); // Restore weather data RTC memory

    // Register WiFi event handlers
    WiFi.onEvent(WiFiStationConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
    WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    Serial.println( "Wifi events done" );

    lv_init();
#if LV_USE_LOG != 0
    lv_log_register_print_cb( my_print ); /* register print function for debugging */
#endif

    tft.begin();          /* TFT init */
    tft.setRotation( 0 ); /* Landscape orientation, flipped */
    
    /*Set the touchscreen calibration data,
    the actual data for your display can be acquired using
    the Generic -> Touch_calibrate example from the TFT_eSPI library*/
    // uint16_t calData[5] = { 275, 3620, 264, 3532, 1 };
    // tft.setTouch( calData );
    touch.begin();
    Serial.println( "tft & touch startet" );

    lv_disp_draw_buf_init( &draw_buf, buf, NULL, screenWidth * screenHeight / 10 );

    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init( &disp_drv );
    /*Change the following line to your display resolution*/
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register( &disp_drv );
    Serial.println( "display initialized" );

    /*Initialize the (dummy) input device driver*/
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init( &indev_drv );
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register( &indev_drv );
    Serial.println( "input device initialized" );
  
    // /* Create simple label */
    // lv_obj_t *label = lv_label_create( lv_scr_act() );
    // lv_label_set_text( label, "Hello Ardino and LVGL!");
    // lv_obj_align( label, LV_ALIGN_CENTER, 0, 0 );

    const esp_timer_create_args_t lvgl_tick_timer_args = {
      .callback = &example_increase_lvgl_tick,
      .name = "lvgl_tick"
    };

    esp_timer_handle_t lvgl_tick_timer = NULL;
    esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer);
    esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000);

    // const esp_timer_create_args_t reboot_timer_args = {
    //   .callback = &example_increase_reboot,
    //   .name = "reboot"
    // };

    // esp_timer_handle_t reboot_timer = NULL;
    // esp_timer_create(&reboot_timer_args, &reboot_timer);
    // esp_timer_start_periodic(reboot_timer, 2000 * 1000);

    screen_on_timer = lv_timer_create(screen_on_timer_cb, display_timeout,  NULL);
    lv_timer_set_repeat_count(screen_on_timer,-1);
    deep_sleep_timer = lv_timer_create(deep_sleep_timer_cb, deep_sleep_timeout, NULL);
    lv_timer_set_repeat_count(deep_sleep_timer,-1);
    Serial.println( "Screen-on & deep-sleep timer created" );
  
    draw_std_scr();
    Serial.println( "screen created" );
  
    QMI8658_init();
    Serial.println( "Setup done" );
}
 
void loop()
{
  lv_timer_handler(); /* let the GUI do its work */
  delay(2);
  if (WiFi.status() == WL_CONNECTED)
  {
    if(weather_status->is_expired())
    {
      Serial.println("Weather data expired, updating...");
      weather_status->update_data();
      Serial.printf("Current temperature:     %.2f°C\n", (weather_status->get_temperature())->get_current());
      Serial.printf("Max temp next 24 hours:  %.2f°C\n", (weather_status->get_temperature())->get_maximum());
      Serial.printf("Min temp next 24 hours:  %.2f°C\n", (weather_status->get_temperature())->get_minimum());
      Serial.printf("Mean temp next 24 hours: %.2f°C\n", (weather_status->get_temperature())->get_mean());
    }
    server.handleClient();
    ElegantOTA.loop();
  }
}
 