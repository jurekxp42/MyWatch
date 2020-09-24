// An Arduino based framework for the Lilygo T-Watch 2020
// Much of the code is based on the sample apps for the
// T-watch that were written and copyrighted by Lewis He.
//(Copyright (c) 2019 lewis he)

// heavily modified by W.F.Dudley Jr.

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/queue.h"
#include <soc/rtc.h>
#include "esp_wifi.h"
#include "esp_sleep.h"
// ---------------
#include "config.h"
#include <SPIFFS.h>	// includes FS.h
// #include <soc/rtc.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include "my_tz.h"
#include "personal_info.h"

#define __MAIN__

#include "DudleyWatch.h"

#define G_EVENT_VBUS_PLUGIN         _BV(0)
#define G_EVENT_VBUS_REMOVE         _BV(1)
#define G_EVENT_CHARGE_DONE         _BV(2)

#define G_EVENT_WIFI_SCAN_START     _BV(3)
#define G_EVENT_WIFI_SCAN_DONE      _BV(4)
#define G_EVENT_WIFI_CONNECTED      _BV(5)
#define G_EVENT_WIFI_BEGIN          _BV(6)
#define G_EVENT_WIFI_OFF            _BV(7)

enum {
    Q_EVENT_WIFI_SCAN_DONE,
    Q_EVENT_WIFI_CONNECT,
    Q_EVENT_BMA_INT,
    Q_EVENT_AXP_INT,
} ;

// #define DEFAULT_SCREEN_TIMEOUT  30*1000 // now user controllable

#define WATCH_FLAG_SLEEP_MODE   _BV(1)
#define WATCH_FLAG_SLEEP_EXIT   _BV(2)
#define WATCH_FLAG_BMA_IRQ      _BV(3)
#define WATCH_FLAG_AXP_IRQ      _BV(4)

QueueHandle_t g_event_queue_handle = NULL;
EventGroupHandle_t g_event_group = NULL;
EventGroupHandle_t isr_group = NULL;
bool lenergy = false;
// void appTouch(void);

uint32_t targetTime = 0;       // for next 1 second display update

void bright_check (void) {
static uint8_t old_brightness;
  if (power->isVBUSPlug()) {
    screen_brightness = 255;
    charge_cable_connected = true;
  }
  else {
    screen_brightness = general_config.default_brightness;
    charge_cable_connected = false;
  }
  if(old_brightness != screen_brightness) {
    ttgo->setBrightness(screen_brightness);       // 0-255
    old_brightness = screen_brightness;
    // ttgo->setBrightness(255);       // 0-255
  }
}

void my_idle(void) {
  last_activity = millis();
  bright_check();
}

void low_energy(void) {
    if (ttgo->bl->isOn()) {
	Serial.println("Entering light sleep mode.");
        xEventGroupSetBits(isr_group, WATCH_FLAG_SLEEP_MODE);
        ttgo->closeBL();
        ttgo->stopLvglTick();
	if(general_config.stepcounter_filter) {
	  ttgo->bma->enableStepCountInterrupt(false);
	}
        ttgo->displaySleep();
        if (!WiFi.isConnected()) {
            lenergy = true;
            WiFi.mode(WIFI_OFF);
            // rtc_clk_cpu_freq_set(RTC_CPU_FREQ_2M);
            setCpuFrequencyMhz(20);

            gpio_wakeup_enable ((gpio_num_t)AXP202_INT, GPIO_INTR_LOW_LEVEL);
            gpio_wakeup_enable ((gpio_num_t)BMA423_INT1, GPIO_INTR_HIGH_LEVEL);
            esp_sleep_enable_gpio_wakeup ();
            esp_light_sleep_start();
        }
    } else {
        ttgo->startLvglTick();
        ttgo->displayWakeup();
        ttgo->rtc->syncToSystem();
	if(general_config.stepcounter_filter) {
	  // updateStepCounter(ttgo->bma->getCounter());
	  step_counter = ttgo->bma->getCounter();
	}
#if LVGL_BATTERY_ICON
        updateBatteryLevel();
        updateBatteryIcon(LV_ICON_CALCULATION);
#endif
        lv_disp_trig_activity(NULL);
        ttgo->openBL();
	if(general_config.stepcounter_filter) {
	  ttgo->bma->enableStepCountInterrupt();
	}
    }
}

void displayTime(uint8_t update_type) {
    bright_check();
    if(general_config.alarm_enable && !alarm_active
    && general_config.alarm_h == local_hour
    && general_config.alarm_m == local_minute) {
	alarm_active = true;
	beep(general_config.alarm_sound);
	next_beep = millis() + 250;
    }
    else if(alarm_active && millis() > next_beep) {
      beep(general_config.alarm_sound);
      next_beep = millis() + 250;
      my_idle();
    }
    if(update_type == 2) {
      date_is_up = false;
      steps_is_up = false;
    }
    switch(chosen_skin) {
	case LCARS :
	    LCARS_Time(update_type);
	    break;
	case ANALOG :
	    Analog_Time(update_type);
	    break;
	case BASIC :
	default :
	    Basic_Time(update_type);
	    break;
    }
}

void Serial_timestamp(void) {
  tnow = ttgo->rtc->getDateTime();
  hh = tnow.hour;
  mm = tnow.minute;
  ss = tnow.second;
  Serial.printf("%d:%02d:%02d UTC\n", hh, mm, ss);
}

void quickBuzz(void) {
  digitalWrite(4, HIGH);
  delay(50);
  digitalWrite(4, LOW);
}

#if NEEDED
void read_block_from_EEPROM (unsigned int start_address, char *block_address, unsigned int sizeof_block) {
  for(unsigned int i = 0 ; i < sizeof_block ; i++) {
    block_address[i] = EEPROM.read(start_address + i);
  }
}

void write_block_to_EEPROM (unsigned int start_address, char *block_address, unsigned int sizeof_block) {
  for(unsigned int i = 0 ; i < sizeof_block ; i++) {
     EEPROM.write(start_address + i, block_address[i]);
  }
  EEPROM.commit();
}
#endif

bool is_sleeping;

void setup() {

  Serial.begin(115200);
  if (!SPIFFS.begin(true)) {
      Serial.println(F("An Error has occurred while mounting SPIFFS"));
  }
#if 0
  bool formatted = SPIFFS.format();
  if(formatted){
      Serial.println(F("Success formatting"));
  }
  else{
      Serial.println(F("Error formatting"));
  }
#endif
  Serial.print("booting DudleyWatch . . . ");
  pinMode(4, OUTPUT);	// vibrator motor pin
  EEPROM.begin(EEPROM_SIZE);
  EEPROM_readAnything(0, general_config); // get saved settings
  if (general_config.magic_number != CONFIG_REVISION ) { //this will set it up for very first use
    Serial.printf("magic wrong, was %ld, should be %ld\n", general_config.magic_number, CONFIG_REVISION);
    general_config.magic_number = CONFIG_REVISION;
    general_config.clock_skin = BASIC;

    general_config.mqtt_server[0] = MQTT_IP0;
    general_config.mqtt_server[1] = MQTT_IP1;
    general_config.mqtt_server[2] = MQTT_IP2;
    general_config.mqtt_server[3] = MQTT_IP3;
    general_config.mqtt_port = MQTT_PORT;

    strcpy_P(general_config.mqtt_user, PSTR(MQTT_USER));
    strcpy_P(general_config.mqtt_pass, PSTR(MQTT_PASS));
    general_config.default_brightness = 128;
    general_config.screensaver_timeout = 30;
    general_config.stepcounter_filter = 1;
    general_config.home_tzindex = MY_TIMEZONE;
    EEPROM_writeAnything(0, general_config);
    EEPROM.commit();
  }
  chosen_skin = general_config.clock_skin;
  tzindex = general_config.local_tzindex;
  //Create a program that allows the required message objects and group flags
  g_event_queue_handle = xQueueCreate(20, sizeof(uint8_t));
  g_event_group = xEventGroupCreate();
  isr_group = xEventGroupCreate();

  ttgo = TTGOClass::getWatch();
  ttgo->begin();
  tft = ttgo->tft;	// just a shorthand for ttgo->tft
  power = ttgo->power;	// just a shorthand for ttgo->power
  tft->setTextFont(1);
  tft->fillScreen(TFT_BLACK);
  tft->setTextColor(TFT_YELLOW, TFT_BLACK); // Note: the new fonts do not draw the background colour
  half_width = tft->width() / 2;
  half_height = tft->height() / 2;

  // Turn on the IRQ used
  power->adc1Enable(AXP202_BATT_VOL_ADC1 | AXP202_BATT_CUR_ADC1 | AXP202_VBUS_VOL_ADC1 | AXP202_VBUS_CUR_ADC1, AXP202_ON);
  power->enableIRQ(AXP202_VBUS_REMOVED_IRQ | AXP202_VBUS_CONNECT_IRQ | AXP202_CHARGING_FINISHED_IRQ, AXP202_ON);
  power->clearIRQ();

  // Turn off unused power
  power->setPowerOutPut(AXP202_EXTEN, AXP202_OFF);
  power->setPowerOutPut(AXP202_DCDC2, AXP202_OFF);
  power->setPowerOutPut(AXP202_LDO3, AXP202_OFF);	// audio device
  power->setPowerOutPut(AXP202_LDO4, AXP202_OFF);

  //Initialize lvgl
  ttgo->lvgl_begin();

#if 1
  // Enable BMA423 interrupt ，
  // The default interrupt configuration,
  // you need to set the acceleration parameters, please refer to the BMA423_Accel example
  ttgo->bma->attachInterrupt();

  //Connection interrupted to the specified pin
  pinMode(BMA423_INT1, INPUT);
  attachInterrupt(BMA423_INT1, [] {
      BaseType_t xHigherPriorityTaskWoken = pdFALSE;
      EventBits_t  bits = xEventGroupGetBitsFromISR(isr_group);
      if (bits & WATCH_FLAG_SLEEP_MODE)
      {
	  //! For quick wake up, use the group flag
	  xEventGroupSetBitsFromISR(isr_group, WATCH_FLAG_SLEEP_EXIT | WATCH_FLAG_BMA_IRQ, &xHigherPriorityTaskWoken);
      } else
      {
	  uint8_t data = Q_EVENT_BMA_INT;
	  xQueueSendFromISR(g_event_queue_handle, &data, &xHigherPriorityTaskWoken);
      }

      if (xHigherPriorityTaskWoken)
      {
	  portYIELD_FROM_ISR ();
      }
  }, RISING);

  // Connection interrupted to the specified pin
  pinMode(AXP202_INT, INPUT);
  attachInterrupt(AXP202_INT, [] {
      BaseType_t xHigherPriorityTaskWoken = pdFALSE;
      EventBits_t  bits = xEventGroupGetBitsFromISR(isr_group);
      if (bits & WATCH_FLAG_SLEEP_MODE)
      {
	  //! For quick wake up, use the group flag
	  xEventGroupSetBitsFromISR(isr_group, WATCH_FLAG_SLEEP_EXIT | WATCH_FLAG_AXP_IRQ, &xHigherPriorityTaskWoken);
      } else
      {
	  uint8_t data = Q_EVENT_AXP_INT;
	  xQueueSendFromISR(g_event_queue_handle, &data, &xHigherPriorityTaskWoken);
      }
      if (xHigherPriorityTaskWoken)
      {
	  portYIELD_FROM_ISR ();
      }
  }, FALLING);
#endif

  // Check if the RTC clock matches, if not, use compile time
  // ttgo->rtc->check();

  // Synchronize time to system time
  ttgo->rtc->syncToSystem();

  Serial_timestamp();
  // Serial.printf("DARKGREY = %x = %d\n", TFT_DARKGREY, TFT_DARKGREY);

#ifdef LILYGO_WATCH_HAS_BUTTON
    /*
        ttgo->button->setClickHandler([]() {
            Serial.println("Button2 Pressed");
        });
    */

    //Set the user button long press to restart
    ttgo->button->setLongClickHandler([]() {
        Serial.println("Pressed Restart Button,Restart now ...");
        delay(1000);
        esp_restart();
    });
#endif
  
  ttgo->openBL(); // Turn on the backlight
  initialAnalog = true;
  displayTime(2); // Full redraw
  is_sleeping = false;
  alarm_active = false;
  next_beep = 0;
  my_idle();
}

void loop(void) {
bool rlst;
uint8_t data;

  //! Fast response wake-up interrupt
  EventBits_t  bits = xEventGroupGetBits(isr_group);
  if (bits & WATCH_FLAG_SLEEP_EXIT) {
      my_idle();
      if (lenergy) {
	lenergy = false;
	// rtc_clk_cpu_freq_set(RTC_CPU_FREQ_160M);
	setCpuFrequencyMhz(160);
      }

      low_energy();

      if (bits & WATCH_FLAG_BMA_IRQ) {
	do {
	  rlst =  ttgo->bma->readInterrupt();
	} while (!rlst);
	xEventGroupClearBits(isr_group, WATCH_FLAG_BMA_IRQ);
      }
      if (bits & WATCH_FLAG_AXP_IRQ) {
	power->readIRQ();
	power->clearIRQ();
	//TODO: Only accept axp power pek key short press
	xEventGroupClearBits(isr_group, WATCH_FLAG_AXP_IRQ);
      }
      xEventGroupClearBits(isr_group, WATCH_FLAG_SLEEP_EXIT);
      xEventGroupClearBits(isr_group, WATCH_FLAG_SLEEP_MODE);
  }
  if ((bits & WATCH_FLAG_SLEEP_MODE)) {
    //! No event processing after entering the information screen
    return;
  }

  //! Normal polling
  if (xQueueReceive(g_event_queue_handle, &data, 5 / portTICK_RATE_MS) == pdPASS) {
    switch (data) {
      case Q_EVENT_BMA_INT:
	do {
	  rlst =  ttgo->bma->readInterrupt();
	} while (!rlst);
	if(general_config.stepcounter_filter) {
	  if (ttgo->bma->isStepCounter()) {
	    // updateStepCounter(ttgo->bma->getCounter());
	    step_counter = ttgo->bma->getCounter();
	  }
	}
	break;
      case Q_EVENT_AXP_INT:
	power->readIRQ();
#if LVGL_BATTERY_ICON
	if (power->isVbusPlugInIRQ()) {
	    updateBatteryIcon(LV_ICON_CHARGE);
	}
	if (power->isVbusRemoveIRQ()) {
	    updateBatteryIcon(LV_ICON_CALCULATION);
	}
	if (power->isChargingDoneIRQ()) {
	    updateBatteryIcon(LV_ICON_CALCULATION);
	}
#endif
	if (power->isPEKShortPressIRQ()) {
	    power->clearIRQ();
	    last_activity = 0;
	    is_sleeping = true;
	    Serial.print("button sleep request ");
	    Serial_timestamp();
	    low_energy();
	    return;
	}
	power->clearIRQ();
	break;
#if NEEDED
      case Q_EVENT_WIFI_SCAN_DONE: {
	int16_t len =  WiFi.scanComplete();
	for (int i = 0; i < len; ++i) {
	  wifi_list_add(WiFi.SSID(i).c_str());
	}
	break;
      }
#endif
      default:
	break;
    }
  }

  // if (lv_disp_get_inactive_time(NULL) < (general_config.screensaver_timeout * 1000))
  if(last_activity + (general_config.screensaver_timeout * 1000) > millis())
  {
    if(is_sleeping) {
      is_sleeping = false;
      Serial.print("wakey-wakey ");
      Serial_timestamp();
      initialAnalog = true;
      displayTime(2); // Full redraw
    }
    else if (targetTime < millis()) {
      targetTime = millis() + 1000;
      // appTouch();
      displayTime((ss == 0) ? 1 : 0); // Call every second but full update every minute
    }

    int mSelect;
    boolean have_run_app = false;
    mSelect = poll_swipe_or_menu_press();	// poll for touch, returns 0-15
    // Serial.printf("p_s... = %d\n", mSelect);
    if (mSelect != -1 && mSelect <= 11) {	// if user touched something
      // this is a bad idea, it's too easy to accidentally
      // start an app and run down the battery.
    }
    else if(mSelect == (DOWN + 11)) {	// swipe is 12 = up, 13 = down, 14, or 15
      // This is where the app selected from the menu is launched
      uint8_t choice = modeMenu();
      if(choice != 0x1b) {
	((void (*)(void))watch_apps[choice].next_menu)();
      }
      have_run_app = true;
    }
    else if(mSelect == (CWCIRCLE + 11)) {
      appMQTT();
      have_run_app = true;
    }
    else if(mSelect == (CCWCIRCLE + 11)) {
      appSettings();
      have_run_app = true;
    }
    else if(mSelect == (RIGHT + 11)) {
      appBattery();
      have_run_app = true;
    }
    else if(mSelect == (LEFT + 11)) {
      if(alarm_active) {
	alarm_active = false;
	next_beep = 0;
      }
      alarmSettings();
      have_run_app = true;
    }
    if(have_run_app) {
      ttgo->tft->fillScreen(TFT_BLACK);
      tft->setTextSize(1);
      initialAnalog = true;
      displayTime(2);
    }
  }
  else {
    if(!is_sleeping) {
      is_sleeping = true;
      Serial.print("sleepy-bye ");
      Serial_timestamp();
    }
    low_energy();
  }
}
