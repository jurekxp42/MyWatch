// Settings
// W.F.Dudley Jr.

#include "config.h"
#include "DudleyWatch.h"
#include <math.h>
#include "my_tz.h"

struct ctrl {
    uint8_t ctype;	// 1 button, 2 slider
    uint16_t xl;
    uint16_t yl;
    uint16_t width;
    uint16_t height;
    uint16_t max;
} control_array[5];

#if LONG_WAY
int control(uint8_t ctype, char *title, int display_var_or_x, int max_var, int yp, int len, int hgt, int slindex) {
  if(ctype == 2) {	// slider
    tft->drawCentreString(title, half_width, yp, 2);
    tft->fillRect( half_width-(len>>1) + 1, yp + 18 + 1, len-2, hgt - 2, TFT_BLACK);
    tft->drawString("0", half_width-(len>>1), yp, 2);
    sprintf(buff, "%d", max_var);
    tft->setTextColor(TFT_RED, TFT_BLACK);
    tft->drawRightString(buff, half_width+(len>>1), yp, 2);
    tft->drawLine( half_width-(len>>1), yp + 18, half_width+(len>>1), yp + 18, TFT_BLUE);	// upper horizontal
    tft->drawLine( half_width-(len>>1), yp + 18 + hgt, half_width+(len>>1), yp + 18 + hgt, TFT_BLUE); // lower horiz
    tft->drawLine( half_width-(len>>1), yp + 18, half_width-(len>>1), yp + 18 + hgt, TFT_BLUE);	// left vertical
    tft->drawLine(half_width+(len>>1), yp + 18, half_width+(len>>1), yp + 18 + hgt, TFT_BLUE); // right vertical
    float sratio = ((float) display_var_or_x) / (float)max_var ;
    int bar_len = (int)(sratio * (float)len) ;
    // Serial.printf("v = %d, ratio = %.2f, len = %d\n", display_var_or_x, sratio, bar_len);
    tft->fillRect( half_width-(len>>1) + 1, yp + 18 + 1, bar_len, hgt - 2, TFT_DARKGREY);
    sprintf(buff, "%d", display_var_or_x);
    tft->drawCentreString(buff, half_width, yp + 28, 2);
    tft->setTextColor(TFT_GREENYELLOW, TFT_BLACK);
  }
  else if(ctype == 1) {		// button
    tft->drawCentreString(title, display_var_or_x + (len >> 1), yp, 2);
    tft->fillRoundRect(display_var_or_x, yp + 18, len, hgt, 15, TFT_DARKGREY);
  }
  if(slindex >= 0) {
    control_array[slindex].ctype = ctype;
    control_array[slindex].xl = display_var_or_x;
    control_array[slindex].yl = yp;
    control_array[slindex].width = len;
    control_array[slindex].height = hgt;
    control_array[slindex].max = max_var;
  }
  return(yp + hgt);
}
#endif

static void page1_create(lv_obj_t * parent);
static void mqtt1_create(lv_obj_t * parent);
static void mqtt2_create(lv_obj_t * parent);
static void tzone_create(lv_obj_t * parent);
static uint8_t event_result;
static int16_t event_value;
static uint8_t slider_num;
static uint8_t button_num;
static uint8_t dropdown_num;

static lv_obj_t *btn1, *btn2, *slider1, *slider2;
static lv_obj_t *slider1_label, *slider2_label;
static lv_obj_t *slider1_name, *slider2_name, *b2label;

static void button_handler(lv_obj_t *obj, lv_event_t event) {
    // Serial.printf("button_handler() event = %d\n", (int)event);
    if (event == LV_EVENT_CLICKED && !event_result) {
        Serial.println(F("Clicked"));
	event_result = 1;
	event_value = 1;
	lv_obj_t * label = lv_obj_get_child(obj, NULL);
	char * txt = lv_label_get_text(label);
	// Serial.printf("button label is %s\n", txt);
	button_num = (strcmp(txt, "Done")) ? 2 : 1 ;
	Serial.printf("handler label = %s, button_num = %d\n", txt, button_num);
	// lv_label_set_text_fmt(label, "Button: %d", cnt);
	return;
    }
    else if (event == LV_EVENT_VALUE_CHANGED && !event_result) {
        Serial.println(F("Toggled"));
	event_result = 1;
	lv_obj_t * label = lv_obj_get_child(obj, NULL);
	char * txt = lv_label_get_text(label);
	// Serial.printf("button label is %s\n", txt);
	button_num = (strcmp(txt, "Done")) ? 2 : 1 ;
	Serial.printf("handler label = %s, button_num = %d\n", txt, button_num);
	// state of toggled button
	event_value = lv_obj_get_state(obj, LV_BTN_PART_MAIN) & LV_STATE_CHECKED;
	Serial.printf("state = %d -> %s\n", event_value, event_value ? "true" : "false");
	return;
    }
}

static void slider_handler(lv_obj_t *obj, lv_event_t event) {
static char buf[4];	// max 3 bytes for number plus 1 null
    // Serial.printf("slider_handler() event = %d\n", (int)event);
    if(event == LV_EVENT_VALUE_CHANGED && !event_result) {
	event_result = 2;
	event_value = lv_slider_get_value(obj);
    }
}

static void slider1_handler(lv_obj_t *obj, lv_event_t event) {
static char buf[4];	// max 3 bytes for number plus 1 null
  slider_handler(obj, event);
  if(event_result == 2) {
    slider_num = 1;
    event_value = lv_slider_get_value(obj);
    snprintf(buf, 4, "%u", event_value);
    lv_label_set_text(slider1_label, buf);
    Serial.printf("handler slider_num = %d, label = %s\n", slider_num, buf);
  }
}

static void slider2_handler(lv_obj_t *obj, lv_event_t event) {
static char buf[4];	// max 3 bytes for number plus 1 null
  slider_handler(obj, event);
  if(event_result) {
    slider_num = 2;
    event_value = lv_slider_get_value(obj);
    snprintf(buf, 4, "%u", event_value);
    lv_label_set_text(slider2_label, buf);
    Serial.printf("handler slider_num = %d, label %s\n", slider_num, buf);
  }
}

static void kb_event_cb1(lv_obj_t * ta, lv_event_t e);
static void kb_event_cb2(lv_obj_t * ta, lv_event_t e);
static void kb_event_cb3(lv_obj_t * ta, lv_event_t e);
static void kb_event_cb4(lv_obj_t * ta, lv_event_t e);
static lv_obj_t * tv;
static lv_obj_t * t1;
static lv_obj_t * t2;
static lv_obj_t * t3;
static lv_obj_t * t4;
static lv_obj_t * kb;
static lv_obj_t * ta1;	// MQTT server IP text area
static lv_obj_t * ta2;	// MQTT port text area
static lv_obj_t * ta3;	// MQTT user text area
static lv_obj_t * ta4;	// MQTT password text area
static lv_style_t style_box;	// is this useful ?

static void ta_event_cb1(lv_obj_t * ta, lv_event_t e) {
    if(e == LV_EVENT_RELEASED) {
        if(kb == NULL) {	// create a keyboard if there isn't one.
	    kb = lv_keyboard_create(lv_scr_act(), NULL);
	    lv_obj_set_size(kb,  LV_HOR_RES, LV_VER_RES / 2);
	    lv_obj_align(kb, NULL, LV_ALIGN_CENTER,  0,  60);
	    lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_NUM);
	    lv_keyboard_set_textarea(kb, ta);
            lv_obj_set_event_cb(kb, kb_event_cb1);
            lv_indev_wait_release(lv_indev_get_act());
        }
        lv_textarea_set_cursor_hidden(ta, false);
        // lv_page_focus(t1, lv_textarea_get_label(ta), LV_ANIM_ON);
        lv_keyboard_set_textarea(kb, ta);
    } else if(e == LV_EVENT_DEFOCUSED) {
        lv_textarea_set_cursor_hidden(ta, true);
    }
}

static void ta_event_cb2(lv_obj_t * ta, lv_event_t e) {
    if(e == LV_EVENT_RELEASED) {
        if(kb == NULL) {	// create a keyboard if there isn't one.
	    kb = lv_keyboard_create(lv_scr_act(), NULL);
	    lv_obj_set_size(kb,  LV_HOR_RES, LV_VER_RES / 2);
	    lv_obj_align(kb, NULL, LV_ALIGN_CENTER,  0, -60);
	    lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_NUM);
	    lv_keyboard_set_textarea(kb, ta);
            lv_obj_set_event_cb(kb, kb_event_cb2);
            lv_indev_wait_release(lv_indev_get_act());
        }
        lv_textarea_set_cursor_hidden(ta, false);
        // lv_page_focus(t1, lv_textarea_get_label(ta), LV_ANIM_ON);
        lv_keyboard_set_textarea(kb, ta);
    } else if(e == LV_EVENT_DEFOCUSED) {
        lv_textarea_set_cursor_hidden(ta, true);
    }
}

static void ta_event_cb3(lv_obj_t * ta, lv_event_t e) {
    if(e == LV_EVENT_RELEASED) {
        if(kb == NULL) {	// create a keyboard if there isn't one.
	    kb = lv_keyboard_create(lv_scr_act(), NULL);
	    lv_obj_set_size(kb,  LV_HOR_RES, LV_VER_RES / 2);
	    lv_obj_align(kb, NULL, LV_ALIGN_CENTER,  0,  60);
	    lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_TEXT_UPPER);
	    lv_keyboard_set_textarea(kb, ta);
            lv_obj_set_event_cb(kb, kb_event_cb3);
            lv_indev_wait_release(lv_indev_get_act());
        }
        lv_textarea_set_cursor_hidden(ta, false);
        // lv_page_focus(t1, lv_textarea_get_label(ta), LV_ANIM_ON);
        lv_keyboard_set_textarea(kb, ta);
    } else if(e == LV_EVENT_DEFOCUSED) {
        lv_textarea_set_cursor_hidden(ta, true);
    }
}

static void ta_event_cb4(lv_obj_t * ta, lv_event_t e) {
    if(e == LV_EVENT_RELEASED) {
        if(kb == NULL) {	// create a keyboard if there isn't one.
	    kb = lv_keyboard_create(lv_scr_act(), NULL);
	    lv_obj_set_size(kb,  LV_HOR_RES, LV_VER_RES / 2);
	    lv_obj_align(kb, NULL, LV_ALIGN_CENTER,  0, -60);
	    lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_TEXT_UPPER);
	    lv_keyboard_set_textarea(kb, ta);
            lv_obj_set_event_cb(kb, kb_event_cb4);
            lv_indev_wait_release(lv_indev_get_act());
        }
        lv_textarea_set_cursor_hidden(ta, false);
        // lv_page_focus(t1, lv_textarea_get_label(ta), LV_ANIM_ON);
        lv_keyboard_set_textarea(kb, ta);
    } else if(e == LV_EVENT_DEFOCUSED) {
        lv_textarea_set_cursor_hidden(ta, true);
    }
}

static void kb_event_cb1(lv_obj_t * _kb, lv_event_t e)
{
    lv_keyboard_def_event_cb(kb, e);

    if(e == LV_EVENT_CANCEL) {
        if(kb) {
            // lv_obj_set_height(tv, LV_VER_RES);
            lv_obj_del(kb);	// delete the keyboard if done.
            kb = NULL;
        }
    }
    if(e == LV_EVENT_APPLY) {
      // announce that the user is finished with the box.
      event_result = 3;
      event_value = 1;
    }
}

static void kb_event_cb2(lv_obj_t * _kb, lv_event_t e)
{
    lv_keyboard_def_event_cb(kb, e);

    if(e == LV_EVENT_CANCEL) {
        if(kb) {
            // lv_obj_set_height(tv, LV_VER_RES);
            lv_obj_del(kb);	// delete the keyboard if done.
            kb = NULL;
        }
    }
    if(e == LV_EVENT_APPLY) {
      // announce that the user is finished with the box.
      event_result = 3;
      event_value = 2;
    }
}

static void kb_event_cb3(lv_obj_t * _kb, lv_event_t e)
{
    lv_keyboard_def_event_cb(kb, e);

    if(e == LV_EVENT_CANCEL) {
        if(kb) {
            // lv_obj_set_height(tv, LV_VER_RES);
            lv_obj_del(kb);	// delete the keyboard if done.
            kb = NULL;
        }
    }
    if(e == LV_EVENT_APPLY) {
      // announce that the user is finished with the box.
      event_result = 3;
      event_value = 3;
    }
}

static void kb_event_cb4(lv_obj_t * _kb, lv_event_t e)
{
    lv_keyboard_def_event_cb(kb, e);

    if(e == LV_EVENT_CANCEL) {
        if(kb) {
            // lv_obj_set_height(tv, LV_VER_RES);
            lv_obj_del(kb);	// delete the keyboard if done.
            kb = NULL;
        }
    }
    if(e == LV_EVENT_APPLY) {
      // announce that the user is finished with the box.
      event_result = 3;
      event_value = 4;
    }
}


static void dd_event_cb1(lv_obj_t * obj, lv_event_t event)
{
  if(event == LV_EVENT_VALUE_CHANGED) {
    char buf[32];
    lv_dropdown_get_selected_str(obj, buf, sizeof(buf));
    printf("Option: %s\n", buf);
    event_result = 4;
    event_value = lv_dropdown_get_selected(obj);
    dropdown_num = 1;
  }
}

static void dd_event_cb2(lv_obj_t * obj, lv_event_t event)
{
  if(event == LV_EVENT_VALUE_CHANGED) {
    char buf[32];
    lv_dropdown_get_selected_str(obj, buf, sizeof(buf));
    printf("Option: %s\n", buf);
    event_result = 4;
    event_value = lv_dropdown_get_selected(obj);
    dropdown_num = 2;
  }
}

void appSettings(void) {
uint32_t lasttouch, interval;
int16_t i, max_bounds, nx, ny, x, y, x0, y0, xmax, ymax, points;
  while(1) {
    max_bounds = 0;
    event_result = 0;
  #if LONG_WAY
    tft->fillScreen(TFT_BLACK);
    tft->setTextColor(TFT_GREENYELLOW, TFT_BLACK);
    tft->drawCentreString("Settings", half_width, 5, 2);

    // slider for screen_brightness goes from 0 to 255
    int ly = control(2, "Screen Brightness", general_config.default_brightness, 255, 30, 220, 40, max_bounds++);
    // slider for screensaver_timeout goes from 0 to 120
    ly = control(2, "Screensaver Timeout", general_config.screensaver_timeout, 120, ly + 28, 220, 40, max_bounds++);
    
    control(1, "Done", 15, 0, ly + 28, 100, 40, max_bounds++);
  #endif
  char buf[10];
    tv = lv_tabview_create(lv_scr_act(), NULL);
    t1 = lv_tabview_add_tab(tv, "Batt");
    t2 = lv_tabview_add_tab(tv, "MQTT\n    1");
    t3 = lv_tabview_add_tab(tv, "MQTT\n    2");
    t4 = lv_tabview_add_tab(tv, "Time\nZone");

    lv_style_init(&style_box);
    lv_style_set_value_align(&style_box, LV_STATE_DEFAULT, LV_ALIGN_OUT_TOP_LEFT);
    lv_style_set_value_ofs_y(&style_box, LV_STATE_DEFAULT, - LV_DPX(10));
    lv_style_set_margin_top(&style_box, LV_STATE_DEFAULT, LV_DPX(30));

    page1_create(t1);
    mqtt1_create(t2);
    mqtt2_create(t3);
    tzone_create(t4);
    while(1) {
      my_idle();
      lv_task_handler();
      delay(5);
      if(event_result) {
	switch(event_result) {
	  case 1 :	// button
	    Serial.printf("eloop: button %d, value = %d\n", button_num, event_value);
	    switch (button_num) {
	      case 1 :
		goto Exit;
	      case 2 :
		general_config.stepcounter_filter = event_value;
		lv_label_set_text(b2label, 
		  (general_config.stepcounter_filter) ? "Step 1" : "Step 0");
		break;
	    }
	    break;
	  case 2 :	// slider
	    Serial.printf("eloop: slider %d, value = %d\n", slider_num, event_value);
	    switch (slider_num) {
	      case 1 :
		general_config.default_brightness = event_value;
		break;
	      case 2 :
		general_config.screensaver_timeout = event_value;
		break;
	    }
	    break;
	  case 3 :	// text box
	    if(event_value == 1) {	// user is done, save result if legal
	      uint16_t ip[4];	// allows us to check for invalid values
	      boolean valid;
	      valid = true;
	      char fkg_buffer[20];
	      char *cp, *sp;
	      const char * txt = lv_textarea_get_text(ta1);
	      strncpy(fkg_buffer, txt, 20);
	      cp = sp = fkg_buffer;
	      Serial.printf("set MQTT server = %s\n", txt);
	      for(int i = 0 ; i < 4 ; i++) {
		while(*cp != '.' && *cp) { cp++; }
		*cp = '\0';
		ip[i] = atoi(sp);
		sp = ++cp;
		if(ip[i] > 255) { valid = false; }
	      }
	      if(valid) {
		Serial.printf("MQTT server ip = %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
		for(int i = 0 ; i < 4 ; i++) {
		  general_config.mqtt_server[i] = ip[i];
		}
	      }
	      else {
		Serial.println(F("Invalid IP address"));
	      }
	    }
	    else if(event_value == 2) {	// user is done, save port # if legal
	      boolean valid;
	      valid = false;
	      uint32_t port;
	      const char * txt = lv_textarea_get_text(ta2);
	      Serial.printf("set MQTT port = %s\n", txt);
	      port = atoi(txt);
	      if(port <= 65535) { valid = true; }
	      if(valid) {
		Serial.printf("MQTT port = %d\n", port);
		general_config.mqtt_port = port;
	      }
	      else {
		Serial.println(F("Invalid port"));
	      }
	    }
	    else if(event_value == 3) {	// user is done, save username
	      const char * txt = lv_textarea_get_text(ta3);
	      Serial.printf("set MQTT user = %s\n", txt);
	      strncpy(general_config.mqtt_user, txt, sizeof(general_config.mqtt_user));
	    }
	    else if(event_value == 4) {	// user is done, save password
	      const char * txt = lv_textarea_get_text(ta4);
	      Serial.printf("set MQTT password = %s\n", txt);
	      strncpy(general_config.mqtt_pass, txt, sizeof(general_config.mqtt_pass));
	    }
	    break;
	  case 4 :	// dropdown
	    if(dropdown_num == 1) {
	      Serial.printf("set LOCAL timezone to index %d -> %d\n", event_value, tz_opts[event_value].tzone);
	      general_config.local_tzindex = tz_opts[event_value].tzone;
	      tzindex = general_config.local_tzindex;
	    }
	    else if(dropdown_num == 2) {
	      Serial.printf("set HOME timezone to index %d -> %d\n", event_value, tz_opts[event_value].tzone);
	      general_config.home_tzindex = tz_opts[event_value].tzone;
	    }
	    break;
	}
	event_result = 0;
      }
  #if LONG_WAY
      if(ttgo->getTouch(nx, ny)) {
	if((nx > 0) && (ny > 0) && (nx < 240) && (ny < 240)) {
	  for(i = 0 ; i < max_bounds ; i++) {
	    if(nx >= control_array[i].xl
	    && nx <= control_array[i].xl+control_array[i].width
	    && ny >= control_array[i].yl
	    && ny <= control_array[i].yl+control_array[i].height) {
	      switch(i) {
		case 0 :
		  y0 = ((float)(nx - control_array[i].xl)/(float)control_array[i].width) * control_array[i].max;
		  if(y0 < 20) { y0 = 20; }
		  Serial.printf("i = %d, nx = %d, ny = %d, yl = %d, yu = %d\n", i, nx,
		    ny, control_array[i].yl, control_array[i].yl+control_array[i].height);
		  Serial.println(F("refreshing control 0"));
		  control(control_array[i].ctype, "Screen Brightness", y0, control_array[i].max, control_array[i].yl, control_array[i].width, 40, -1);
		  general_config.default_brightness = y0;
		  break;
		case 1 :
		  y0 = ((float)(nx - control_array[i].xl)/(float)control_array[i].width) * control_array[i].max;
		  if(y0 < 5) { y0 = 5; }
		  Serial.printf("i = %d, nx = %d, ny = %d, yl = %d, yu = %d\n", i, nx,
		    ny, control_array[i].yl, control_array[i].yl+control_array[i].height);
		  Serial.println(F("refreshing control 1"));
		  control(control_array[i].ctype, "Screensaver Timeout", y0, control_array[i].max, control_array[i].yl, control_array[i].width, 40, -1);
		  general_config.screensaver_timeout = y0;
		  break;
		case 2 :
		  y0 = ((float)(nx - control_array[i].xl)/(float)control_array[i].width) * control_array[i].max;
		  Serial.printf("i = %d, nx = %d, ny = %d, yl = %d, yu = %d\n", i, nx,
		    ny, control_array[i].yl, control_array[i].yl+control_array[i].height);
		  // Serial.println(F("refreshing control 2"));
		  // control(control_array[i].ctype, "Done", 30, 0, control_array[i].yl, control_array[i].width, 40, -1);
		  goto Exit;
	      }
	    }
	  }
	}
      }
  #endif
    }
  }
  do {
    // wait for a touch
    while (!ttgo->getTouch(x, y)) {
      my_idle();
    }
    // Serial.printf("y = %d\n", y);
    // wait for end of touch
    while (ttgo->getTouch(x0, y0)) {
      my_idle();
    }
  } while(y < 0 || y >= 240);
Exit:
  EEPROM_writeAnything(0, general_config);	// save the setting changes
  EEPROM.commit();
  Serial.println(F("Exit appSettings()"));
  ttgo->tft->fillScreen(TFT_BLACK);
}

static void page1_create(lv_obj_t * parent) {
static char buf[4];	// max 3 bytes for number plus 1 null
  lv_obj_t *label;
  lv_page_set_scrl_layout(parent, LV_LAYOUT_PRETTY_TOP);
  lv_disp_size_t disp_size = lv_disp_get_size_category(NULL);
  // LV_DISP_SIZE_SMALL = disp_size = 0 !
  // lv_coord_t grid_w = lv_page_get_width_grid(parent, disp_size <= LV_DISP_SIZE_SMALL ? 1 : 2, 1);
  lv_obj_t * h = lv_cont_create(parent, NULL);
  lv_obj_set_width(h, LV_DPI * 2);
  lv_obj_set_height(h, 200);

  lv_obj_t * slider1 = lv_slider_create(h, NULL);
  lv_obj_set_width(slider1, 200);
  lv_obj_align(slider1, NULL, LV_ALIGN_CENTER, -20, -70);
  lv_obj_set_event_cb(slider1, slider1_handler);
  lv_slider_set_range(slider1, 20, 255);
  lv_slider_set_value(slider1, general_config.default_brightness, LV_ANIM_OFF);
  /* Create a label below the slider */
  slider1_label = lv_label_create(h, NULL);
  snprintf(buf, 4, "%u", lv_slider_get_value(slider1));
  lv_label_set_text(slider1_label, buf);
  lv_obj_set_auto_realign(slider1_label, true);
  lv_obj_align(slider1_label, slider1, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
  slider1_name = lv_label_create(h, NULL);
  lv_label_set_text(slider1_name, "screen brightness");
  lv_obj_set_auto_realign(slider1_name, true);
  lv_obj_align(slider1_name, slider1, LV_ALIGN_OUT_TOP_MID, 0, -5);

  lv_obj_t * slider2 = lv_slider_create(h, NULL);
  lv_obj_set_width(slider2, 200);
  lv_obj_align(slider2, NULL, LV_ALIGN_CENTER, -20, -25);
  lv_obj_set_event_cb(slider2, slider2_handler);
  lv_slider_set_range(slider2, 5, 120);
  lv_slider_set_value(slider2, general_config.screensaver_timeout, LV_ANIM_OFF);
  /* Create a label below the slider */
  slider2_label = lv_label_create(h, NULL);
  snprintf(buf, 4, "%u", lv_slider_get_value(slider2));
  lv_label_set_text(slider2_label, buf);
  lv_obj_set_auto_realign(slider2_label, true);
  lv_obj_align(slider2_label, slider2, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
  slider2_name = lv_label_create(h, NULL);
  lv_label_set_text(slider2_name, "screenblank time");
  lv_obj_set_auto_realign(slider2_name, true);
  lv_obj_align(slider2_name, slider2, LV_ALIGN_OUT_TOP_MID, 0, -5);

  lv_obj_t *btn1 = lv_btn_create(h, NULL);
  lv_obj_set_event_cb(btn1, button_handler);
  lv_obj_align(btn1, NULL, LV_ALIGN_CENTER, -65, 32);

  label = lv_label_create(btn1, NULL);
  lv_obj_set_size(btn1, 100, 50); //set the button size
  lv_label_set_text(label, "Done");

  lv_obj_t *btn2 = lv_btn_create(h, NULL);
  lv_obj_set_event_cb(btn2, button_handler);
  lv_obj_align(btn2, NULL, LV_ALIGN_CENTER, 55, 32);
  lv_btn_set_checkable(btn2, true);
  lv_obj_set_size(btn2, 100, 50); //set the button size
  lv_btn_toggle(btn2);
  // lv_btn_set_fit2(btn2, LV_FIT_NONE, LV_FIT_TIGHT);
  b2label = lv_label_create(btn2, NULL);
  lv_label_set_text(b2label, 
	(general_config.stepcounter_filter) ? "Step 1" : "Step 0");

}

static void mqtt1_create(lv_obj_t * parent) {
static char buf[4];	// max 3 bytes for number plus 1 null
  lv_obj_t *label;
  lv_page_set_scrl_layout(parent, LV_LAYOUT_PRETTY_TOP);
  lv_obj_t * h = lv_cont_create(parent, NULL);
  lv_obj_set_width(h, LV_DPI * 2);
  lv_obj_set_height(h, 200);

  lv_obj_t *btn1 = lv_btn_create(h, NULL);
  lv_obj_set_event_cb(btn1, button_handler);
  lv_obj_align(btn1, NULL, LV_ALIGN_CENTER, -65, 32);

  label = lv_label_create(btn1, NULL);
  lv_obj_set_size(btn1, 100, 50); //set the button size
  lv_label_set_text(label, "Done");

  /* Create the text area for MQTT server IP */
  ta1 = lv_textarea_create(h, NULL);
  lv_obj_set_event_cb(ta1, ta_event_cb1);
  lv_obj_set_width(ta1, 150);
  lv_obj_align(ta1, NULL, LV_ALIGN_CENTER, -25, -30);	// order of operations matters!
  lv_textarea_set_accepted_chars(ta1, "0123456789.");
  lv_textarea_set_max_length(ta1, 15);
  lv_textarea_set_one_line(ta1, true);
  sprintf(buff, "%d.%d.%d.%d",
    general_config.mqtt_server[0], general_config.mqtt_server[1],
    general_config.mqtt_server[2], general_config.mqtt_server[3]);
  lv_textarea_set_text(ta1, buff);
  lv_obj_t *ta1_name = lv_label_create(h, NULL);
  lv_label_set_text(ta1_name, "MQTT server IP");
  lv_obj_set_auto_realign(ta1_name, true);
  lv_obj_align(ta1_name, ta1, LV_ALIGN_OUT_BOTTOM_MID, 0,  0);

  /* Create the text area for MQTT port */
  ta2 = lv_textarea_create(h, NULL);
  lv_obj_set_event_cb(ta2, ta_event_cb2);
  lv_obj_set_width(ta2, 150);
  lv_obj_align(ta2, NULL, LV_ALIGN_CENTER, -25, 20);	// order of operations matters!
  lv_textarea_set_accepted_chars(ta2, "0123456789");
  lv_textarea_set_max_length(ta2, 6);
  lv_textarea_set_one_line(ta2, true);
  sprintf(buff, "%d", general_config.mqtt_port);
  lv_textarea_set_text(ta2, buff);
  lv_obj_t *ta2_name = lv_label_create(h, NULL);
  lv_label_set_text(ta2_name, "MQTT port");
  lv_obj_set_auto_realign(ta2_name, true);
  lv_obj_align(ta2_name, ta2, LV_ALIGN_OUT_BOTTOM_MID, 0,  0);
}

static void mqtt2_create(lv_obj_t * parent) {
  lv_obj_t *label;
  lv_page_set_scrl_layout(parent, LV_LAYOUT_PRETTY_TOP);
  lv_obj_t * h = lv_cont_create(parent, NULL);
  lv_obj_set_width(h, LV_DPI * 2);
  lv_obj_set_height(h, 200);

  lv_obj_t *btn1 = lv_btn_create(h, NULL);
  lv_obj_set_event_cb(btn1, button_handler);
  lv_obj_align(btn1, NULL, LV_ALIGN_CENTER, -65, 32);

  label = lv_label_create(btn1, NULL);
  lv_obj_set_size(btn1, 100, 50); //set the button size
  lv_label_set_text(label, "Done");

  /* Create the text area for MQTT user */
  ta3 = lv_textarea_create(h, NULL);
  lv_obj_set_event_cb(ta3, ta_event_cb3);
  lv_obj_set_width(ta3, 150);
  lv_obj_align(ta3, NULL, LV_ALIGN_CENTER, -25, -30);	// order of operations matters!
  lv_textarea_set_max_length(ta3, 20);
  lv_textarea_set_one_line(ta3, true);
  // Serial.printf("mqtt_user was %s\n", general_config.mqtt_user);
  // strcpy(general_config.mqtt_user, "OBS27-27A");
  sprintf(buff, "%s", general_config.mqtt_user);
  lv_textarea_set_text(ta3, buff);
  lv_obj_t *ta3_name = lv_label_create(h, NULL);
  lv_label_set_text(ta3_name, "MQTT user");
  lv_obj_set_auto_realign(ta3_name, true);
  lv_obj_align(ta3_name, ta3, LV_ALIGN_OUT_BOTTOM_MID, 0,  0);

  /* Create the text area for MQTT password */
  ta4 = lv_textarea_create(h, NULL);
  lv_obj_set_event_cb(ta4, ta_event_cb4);
  lv_obj_set_width(ta4, 150);
  lv_obj_align(ta4, NULL, LV_ALIGN_CENTER, -25, 20);	// order of operations matters!
  lv_textarea_set_max_length(ta4, 20);
  lv_textarea_set_one_line(ta4, true);
  sprintf(buff, "%s", general_config.mqtt_pass);
  lv_textarea_set_text(ta4, buff);
  lv_obj_t *ta4_name = lv_label_create(h, NULL);
  lv_label_set_text(ta4_name, "MQTT password");
  lv_obj_set_auto_realign(ta4_name, true);
  lv_obj_align(ta4_name, ta4, LV_ALIGN_OUT_BOTTOM_MID, 0,  0);
}

static void tzone_create(lv_obj_t * parent) {
char buf[32];
int selected;
  lv_obj_t *label;
  lv_page_set_scrl_layout(parent, LV_LAYOUT_PRETTY_TOP);
  lv_obj_t * h = lv_cont_create(parent, NULL);
  lv_obj_set_width(h, LV_DPI * 2);
  lv_obj_set_height(h, 200);

  lv_obj_t *btn1 = lv_btn_create(h, NULL);
  lv_obj_set_event_cb(btn1, button_handler);
  lv_obj_align(btn1, NULL, LV_ALIGN_CENTER, -65, 32);

  label = lv_label_create(btn1, NULL);
  lv_obj_set_size(btn1, 100, 50); //set the button size
  lv_label_set_text(label, "Done");

  /* Create the dropdown for local timezone */
  lv_obj_t * dd1 = lv_dropdown_create(h, NULL);
  lv_obj_add_style(dd1, LV_CONT_PART_MAIN, &style_box);
  lv_obj_set_style_local_value_str(dd1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "   Local TZ");
  // lv_obj_set_width(dd1, lv_obj_get_width_grid(h, disp_size <= LV_DISP_SIZE_SMALL ? 1 : 2, 1));
  lv_obj_align(dd1, NULL, LV_ALIGN_CENTER, -70, -40);
  lv_obj_set_width(dd1, 110);
  buff[0] = '\0';
  // Serial.printf("sizeof_tz_opts = %d, local_tzindex = %d\n", sizeof_tz_opts, general_config.local_tzindex);
  for(int i = 0 ; i < sizeof_tz_opts ; i++) {
    strcat(buff, tz_opts[i].tzname);
    if(i < sizeof_tz_opts - 1) {
      strcat(buff, "\n");
    }
    if(tz_opts[i].tzone == general_config.local_tzindex) {
      // Serial.printf("home_tzindex, found match i = %d, value %s\n", i, tz_opts[i].tzname);
      selected = i;
    }
  }
  lv_dropdown_set_options(dd1, buff);
  lv_dropdown_set_selected(dd1, selected);
  lv_obj_set_event_cb(dd1, dd_event_cb1);

  /* Create the dropdown for home timezone */
  lv_obj_t * dd2 = lv_dropdown_create(h, NULL);
  lv_obj_add_style(dd2, LV_CONT_PART_MAIN, &style_box);
  lv_obj_set_style_local_value_str(dd2, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "   Home TZ");
  // lv_obj_set_width(dd2, lv_obj_get_width_grid(h, disp_size <= LV_DISP_SIZE_SMALL ? 1 : 2, 1));
  lv_obj_align(dd2, NULL, LV_ALIGN_CENTER,  40, -40);
  lv_obj_set_width(dd2, 110);
  buff[0] = '\0';
  // Serial.printf("sizeof_tz_opts = %d, home_tzindex = %d\n", sizeof_tz_opts, general_config.home_tzindex);
  for(int i = 0 ; i < sizeof_tz_opts ; i++) {
    strcat(buff, tz_opts[i].tzname);
    if(i < sizeof_tz_opts - 1) {
      strcat(buff, "\n");
    }
    if(tz_opts[i].tzone == general_config.home_tzindex) {
      // Serial.printf("home_tzindex, found match i = %d, value %s\n", i, tz_opts[i].tzname);
      selected = i;
    }
  }
  lv_dropdown_set_options(dd2, buff);
  lv_dropdown_set_selected(dd2, selected);
  lv_obj_set_event_cb(dd2, dd_event_cb2);
}
