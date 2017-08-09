# CO2-Data-Logger-MHZ14
Arduino library created for the purpose of CO2 ppm detection and data logging using the MHZ14 sensor and Adafruit's BME280 Temp sensor and data logger.

This code runs every 90 seconds but the parameters can be changed under #define LOG_INTERVAL 90000. Note 90000 = 90 seconds so 1000 = 1 sec.

The CO2 sensor is fairly accurate for readings and calibration should happen no less than every 6 months. 
