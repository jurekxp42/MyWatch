#ifndef __APP_MQTT_H__
#define __APP_MQTT_H__

EXTERN struct menu_item sensor_menu1[]
#ifdef __MQTT_MAIN__
  = {
      
      { "CO2", "esp8266/co2", NULL},
      { "Temp", "esp8266/sensor1_temperature_1", NULL},
      { "", "", NULL},
      { "", "", NULL},
      { "", "", NULL},
      { "", "", NULL},
      { "", "", NULL},
      { "", "", NULL},
      { "", "", NULL},
      { "", "", NULL},
      { "",    "", NULL},
      { "Exit",      "", NULL}
    }
#endif
    ;

#endif
