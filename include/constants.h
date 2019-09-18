#ifndef _WEATHERMAN_CONFIG_H_
#define _WEATHERMAN_CONFIG_H_

#define PIN_LED         5
#define PIN_ANEMOMETER  33  // Black wire
#define PIN_WINDVANE    35  // Grey wire
#define PIN_RAINGAUGE   13  // White wire


#define DELAY_1         1000
#define DELAY_2         1000
#define BLINK_IOT_FAIL  9


#define _MIN_DELTA_TEMPERATURE  0.25f   // report temperature after change of 0.25 °C
#define _MIN_DELTA_PRESSURE     50      // report pressure after change of 0.5 hPa
#define _MIN_DELTA_LUMINOSITY   25      // report luminosity after change of 25 lux
#define _MIN_DELTA_HUMIDITY     0.5f    // report humidity after change of 0.5 %
#define _MIN_DELTA_WIND_SPEED   1       // report wind speed after change of 1 km/h
#define _MIN_DELTA_RAIN_LEVEL   1       // report rain level after change of 1 mm/h


#define MQTT_MAIN_TOPIC  "weatherman"
#define CONFIG_DEVICE_ID "DeviceId"

#endif