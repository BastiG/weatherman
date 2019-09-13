#ifndef _WEATHERMAN_READING_H_
#define _WEATHERMAN_READING_H_

#define _MIN_DELTA_TEMPERATURE  0.25f   // report temperature after change of 0.25 °C
#define _MIN_DELTA_PRESSURE     50      // report pressure after change of 0.5 hPa
#define _MIN_DELTA_LUMINOSITY   25      // report luminosity after change of 25 lux
#define _MIN_DELTA_HUMIDITY     0.5f    // report humidity after change of 0.5 %
#define _MIN_DELTA_WIND_SPEED   1       // report wind speed after change of 1 km/h
#define _MIN_DELTA_RAIN_LEVEL   1       // report rain level after change of 1 mm/h


void readTemperature(void);
void readPressure(void);
void readLuminosity(void);
void readHumidity(void);
void readWindSpeed(void);
void readRainLevel(void);
void readWindDirection(void);


void resetTemperature(void);
void resetPressure(void);
void resetLuminosity(void);
void resetHumidity(void);
void resetWindSpeed(void);
void resetRainLevel(void);
void resetWindDirection(void);

#endif