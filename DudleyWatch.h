/*
  change log
  2020 09 12 -- added step display to all clocks, re-worked step icon to be
  re-sizeable, made step display code take arguments.
  Added converting char * IP address to array of four bytes.  Added saving
  config changes to EEPROM in appSettings.

  2020 09 23 -- added sound and alarm values to general_config
  2020 12 10 -- added appWeather()
  2020 12 18 -- added alerts to appWeather()
  2020 12 19 -- added weatherSetup()

  W.F.Dudley Jr.
*/
#ifndef DUDLEY_WATCH_H
#define DUDLEY_WATCH_H

#include <EEPROM.h>
#include "EEPROM_rw_anything.h"

#ifdef __MAIN__
#define EXTERN
#define INIT(x) = x
#else
#define EXTERN extern
#define INIT(x)
#endif

struct menu_item{
  char *name;
  char *topic;
  void* next_menu;
};

void LilyGo_Time(uint8_t);
boolean if_not_home_tz(void);

EXTERN uint8_t home_hh;
EXTERN uint8_t home_mm;
EXTERN uint8_t home_ss;
EXTERN RTC_Date tnow;
EXTERN uint8_t local_hour;
EXTERN uint8_t local_minute;
EXTERN boolean rtcIrq;

// this is neat, but we store chosen_skin in general_config, so . . .
// EXTERN void (*watch_fun_pointer)(uint8_t);

EXTERN struct menu_item skin_menu[]
#ifdef __MAIN__
  = {
      { "LilyGo", "", (void *)&LilyGo_Time }
    }
#endif
    ;

// skins:
#define LILYGO 4

uint8_t modeMenu(void);	// app selector

void appMaze(void);		// Maze game/demo
void appSetTime(void);		// set clock from keyboard
void appSettings(void);		// settings
void displayTime(uint8_t);	// watch "wrapper" app
void jSats(void);		// Jupiter satellites app
void appLevel(void);		// Bubble Level (accel test)
void appBattery(void);		// "all" battery info
void appNTPTime(void);		// set time using NTP
void appMQTT(void);		// MQTT app
void appLife(void);		// Conway's game of life
void appPaint(void);		// paint program
void skinMenu(void);		// set clock style ("skin")
void appStopWatch(void);	// stop watch
void alarmSettings(void);	// alarm clock settings
void appCalculator(void);	// calculator
void appCalendar(void);		// calendar
void switch_menu(void);		// switch apps menu page
void appBitcoin(void);		// Bitcoin value checker
void appMandelbrot(void);	// Mandelbrot generator
void resetStepCounter(void);	// like it says on the tin
void appWeather(void);		// get the weather
void appTouch(void);		// touch screen test
void appDelWiFi(void);		// delete one SSID from acc_pts.txt
void weatherSettings(void);	// settings page for appWeather
void appWiFiScan(void);		// wifi scanner tool
void appColorPicker(void);
void appShutdown(void);

void charge_icon (uint16_t, uint16_t, uint16_t, uint16_t, boolean);
void battery_icon(uint16_t, uint16_t, uint16_t, uint16_t);
void steps_icon (uint16_t, uint16_t, uint16_t, uint16_t, uint8_t);
void draw_step_counter_ljust (uint16_t, uint16_t, uint16_t, uint16_t, uint8_t);
void draw_step_counter_rjust (uint16_t, uint16_t, uint16_t, uint16_t, uint8_t);
void update_step_counter(void);

void quickBuzz(void);
void beep(int8_t);
void draw_keyboard(uint8_t, const char **, uint8_t, bool, char *);
void flash_keyboard_item (const char **, uint8_t, bool, int, int);
int get_numerical_button_press(void);
void dummyTime(void);
void bright_check (void);
void my_idle(void);
void draw_button_menu (uint8_t, struct menu_item *, uint8_t, uint8_t, bool, char *, struct mqdata *, uint8_t);
void flash_menu_item (uint8_t, struct menu_item *, uint8_t, uint8_t, bool, int, int, struct mqdata *, boolean, uint8_t);
void flash_menu_item_txt (uint8_t, struct menu_item *, uint8_t, uint8_t, bool, int, int, char *, boolean, uint8_t);
void enable_rtc_alarm(void);
void disable_rtc_alarm(void);
void Serial_timestamp(void);
int cvt_12_hour_clock(int);

EXTERN boolean date_is_up;
EXTERN boolean steps_is_up;
EXTERN boolean initialAnalog;
EXTERN char buff[512];
EXTERN unsigned int half_width;
EXTERN unsigned int half_height;
EXTERN TFT_eSPI *tft;
EXTERN AXP20X_Class *power;
EXTERN uint8_t screen_brightness;
EXTERN boolean charge_cable_connected;
extern uint8_t number_of_sounds; // leave lower case, defined in play_sound.cpp
EXTERN boolean alarm_active;
EXTERN uint32_t next_beep;	// the next time the alarm beep should sound
EXTERN struct menu_item *app_menu_ptr; // which watch_apps menu is in current use?
EXTERN const char **app_label_ptr; // which app_labels array is in current use?
EXTERN const char *last_app_name;	// name of last app that was selected

EXTERN struct menu_item watch_apps[]
#ifdef __MAIN__
= {
    { "Timer",  "", (void *)&appStopWatch },
    { "Wetter",     "", (void *)&appWeather },
    { "Bat",     "", (void *)&appBattery },
    { "Rechner",  "", (void *)&appCalculator },
    { "Laby",        "", (void *)&appMaze },
    { "MQTT", "", (void *)&appMQTT },
    { "M-brot",  "", (void *)&appMandelbrot },
    { "Malen",       "", (void *)&appPaint },
    { "Kalender",    "", (void *)&appCalendar },
    { "Touch",  "", (void *)&appTouch },
    { "Config",      "", (void *)&switch_menu },
    { "Back",       "", NULL }
}
#endif
;

EXTERN const char *app_labels[]
#ifdef __MAIN__
		      = {
			  watch_apps[0].name, watch_apps[1].name, watch_apps[2].name,
			  watch_apps[3].name, watch_apps[4].name, watch_apps[5].name,
			  watch_apps[6].name, watch_apps[7].name, watch_apps[8].name,
			  watch_apps[9].name, watch_apps[10].name, watch_apps[11].name
			}
#endif
;

EXTERN struct menu_item watch_apps2[]
#ifdef __MAIN__
= {
    { "WiFi Scan",   "", (void *)appWiFiScan },
    
    { "NTP",    "", (void *)&appNTPTime },    
    { "Wecker",  "", (void *)&alarmSettings },
    { "Einst.",    "", (void *)&appSettings },
    { "Del WiFi", "", (void *)&appDelWiFi },
    { "Leben",        "", (void *)&appLife },
    { "Accel",       "", (void *)&appLevel },
    { "Farbe", "", (void *)&appColorPicker },
    { "Zeit", "", (void *)&appSetTime },
    { "Wetter", "", (void *)weatherSettings },
    { "Apps",      "", (void *)&switch_menu },
    { "Back",       "", NULL }
}
#endif
;

EXTERN const char *app_labels2[]
#ifdef __MAIN__
		      = {
			  watch_apps2[0].name, watch_apps2[1].name, watch_apps2[2].name,
			  watch_apps2[3].name, watch_apps2[4].name, watch_apps2[5].name,
			  watch_apps2[6].name, watch_apps2[7].name, watch_apps2[8].name,
			  watch_apps2[9].name, watch_apps2[10].name, watch_apps2[11].name
			}
#endif
;


EXTERN TTGOClass *ttgo;
EXTERN uint8_t hh, mm, ss, mmonth, dday, gdow; // H, M, S variables
EXTERN uint16_t yyear; // Year is 16 bit int
EXTERN uint32_t last_activity;
EXTERN uint8_t chosen_skin;
EXTERN int step_counter, last_step_counter;

#define CONFIG_REVISION 12349L
typedef struct config_t {
    long magic_number;
    uint8_t clock_skin;
    uint8_t mqtt_server[4];
    char mqtt_user[20];
    char mqtt_pass[20];
    uint16_t mqtt_port;
    uint8_t default_brightness;
    uint8_t screensaver_timeout;	// in seconds
    uint8_t stepcounter_filter;		// 0 to disable, 1,2,3,4 for filters
    uint32_t home_tzindex;
    uint32_t local_tzindex;
    boolean alarm_enable;
    uint8_t alarm_h;
    uint8_t alarm_m;
    int8_t alarm_days[7];
    int8_t alarm_sound;
    int8_t alarm_volume;
    int8_t twelve_hr_clock;
    boolean metric_units;
    char owm_api_key[40];
    char language[6];
    char my_latitude[20];
    char my_longitude[20];
} CONFIGGEN;

EXTERN CONFIGGEN general_config;

enum LV_THING { NILEVENT=0, BUTTON, SLIDER, KEYBOARD, DROPDOWN, SWITCH };

enum SWIPE_DIR { NODIR=31, UP, DOWN, LEFT, RIGHT, CWCIRCLE, CCWCIRCLE };

EXTERN const char *swipe_names[]
#ifdef __MAIN__
= { "nodir", "up", "down", "left", "right" }
#endif
;

int poll_swipe_or_menu_press(int);

#define DL(x) Serial.print(x)
#define DLn(x) Serial.println(x)
#define DLF(x) Serial.print(F(x))
#define DLFn(x) Serial.println(F(x))
#define DV(m, v) do{Serial.printf("%s %s", m, v);}while(0)
// #define DV(m, v) do{Serial.print(m);Serial.print(v);Serial.print(" ");}while(0)
#define DVn(m, v) do{Serial.printf("%s %s\n", m, v);}while(0)
// #define DVn(m, v) do{Serial.print(m);Serial.println(v);}while(0)
#define DVF(m, v) do{Serial.print(F(m));Serial.print(v);Serial.print(F(" "));}while(0)
#define DVFn(m, v) do{Serial.print(F(m));Serial.println(v);}while(0)

#endif
