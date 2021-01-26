/**
 * Simple clock display interface
 * Written by lewishe
 * */

#include <AceTime.h>
#include "config.h"
#include "DudleyWatch.h"
#include "my_tz.h"

using namespace ace_time;

int per;
float temp;

int cvt_12_hour_clock(int h24) {
int h12;
  if(general_config.twelve_hr_clock) {
    h12 = h24 % 12;
    if(h12 == 0) { h12 = 12; }
    return h12;
  }
  return h24;
}

LV_IMG_DECLARE(arrow_left_png);
LV_IMG_DECLARE(arrow_right_png);
LV_FONT_DECLARE(fn1_32);
LV_FONT_DECLARE(robot_ightItalic_16);
LV_FONT_DECLARE(robot_light_16);
LV_FONT_DECLARE(liquidCrystal_nor_64);
LV_FONT_DECLARE(liquidCrystal_nor_32);
LV_FONT_DECLARE(liquidCrystal_nor_24);
LV_FONT_DECLARE(digital_play_st_48);
LV_FONT_DECLARE(quostige_16);
LV_FONT_DECLARE(digital_play_st_24);
LV_FONT_DECLARE(gracetians_32);
LV_FONT_DECLARE(exninja_22);

static lv_obj_t *hours, *minute, *second, *day, *dow, *month, *year, *am_pm;
static lv_obj_t *bat, *temp_text, *str2, *model, *batt, *indicator;

lv_obj_t *setupGUI(void) {
    static lv_style_t cont_style;
    lv_style_init(&cont_style);
    lv_style_init(&cont_style);
    lv_style_set_radius(&cont_style, LV_OBJ_PART_MAIN, 0);
    lv_style_set_bg_color(&cont_style, LV_OBJ_PART_MAIN, LV_COLOR_BLACK);
    lv_style_set_bg_opa(&cont_style, LV_OBJ_PART_MAIN, LV_OPA_COVER);
    lv_style_set_border_width(&cont_style, LV_OBJ_PART_MAIN, 0);

    lv_obj_t *view = lv_cont_create(lv_scr_act(), nullptr);
    lv_obj_set_size(view, 240, 240);
    lv_obj_add_style(view, LV_OBJ_PART_MAIN, &cont_style);

    static lv_style_t onestyle;
    lv_style_init(&onestyle);
    lv_style_set_text_color(&onestyle, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_style_set_text_font(&onestyle, LV_STATE_DEFAULT, &fn1_32);

    // Upper left corner logo
    lv_obj_t *casio = lv_label_create(view, nullptr);
    lv_obj_add_style(casio, LV_OBJ_PART_MAIN, &onestyle);
    lv_label_set_text(casio, "HAL");
    lv_obj_align(casio, view, LV_ALIGN_IN_TOP_LEFT, 10, 10);

    // Upper right corner model -> now "charging" indicator
    static lv_style_t model_style;
    lv_style_init(&model_style);
    lv_style_set_text_color(&model_style, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_style_set_text_font(&model_style, LV_STATE_DEFAULT, &robot_ightItalic_16);

    batt = lv_label_create(view, nullptr);
    lv_obj_add_style(batt, LV_OBJ_PART_MAIN, &model_style);
    lv_label_set_text(batt, (charge_cable_connected) ? "CHARGING" : "bat");
    lv_obj_align(batt, view, LV_ALIGN_IN_TOP_RIGHT, 20, 15);

    indicator = lv_label_create(view, nullptr);
    lv_obj_add_style(indicator, LV_OBJ_PART_MAIN, &model_style);
    lv_label_set_text(indicator, "C" );
    lv_obj_align(indicator, view, LV_ALIGN_IN_TOP_RIGHT, -5, 15);

    //Line style
    static lv_style_t line_style;
    lv_style_init(&line_style);
    lv_style_set_line_color(&line_style, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_style_set_line_width(&line_style, LV_STATE_DEFAULT, 2);
    lv_style_set_line_rounded(&line_style, LV_STATE_DEFAULT, 1);

    //Top horizontal line
    static lv_point_t line_points[] = { {10, 0}, {230, 0} };
    lv_obj_t *line1;
    line1 = lv_line_create(view, NULL);
    lv_line_set_points(line1, line_points, 2);     /*Set the points*/
    lv_obj_add_style(line1, LV_OBJ_PART_MAIN, &line_style);
    lv_obj_align(line1, NULL, LV_ALIGN_IN_TOP_MID, 0, 45);

    //Slogan below the top horizontal line
    lv_obj_t *row_down_line = lv_label_create(view, nullptr);
    lv_obj_add_style(row_down_line, LV_OBJ_PART_MAIN, &model_style);
    lv_label_set_text(row_down_line, "9000");
    lv_obj_align(row_down_line, line1, LV_ALIGN_OUT_BOTTOM_LEFT, 10, 2);

    // bottom left horizontal line
    static lv_point_t line_points1[] = { {0, 0}, {230, 0} };
    lv_obj_t *line2;
    line2 = lv_line_create(view, NULL);
    lv_line_set_points(line2, line_points1, 2);     /*Set the points*/
    lv_obj_add_style(line2, LV_OBJ_PART_MAIN, &line_style);
    lv_obj_align(line2, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 10, -30);

    // Below the horizontal line in the upper left corner
    static lv_style_t text_style;
    lv_style_init(&text_style);
    lv_style_set_text_color(&text_style, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_style_set_text_font(&text_style, LV_STATE_DEFAULT, &robot_ightItalic_16);

    //Intermediate clock time division font
    static lv_style_t time_style;
    lv_style_init(&time_style);
    lv_style_set_text_color(&time_style, LV_STATE_DEFAULT, LV_COLOR_SILVER);
    lv_style_set_text_font(&time_style, LV_STATE_DEFAULT, &digital_play_st_48);

    hours = lv_label_create(view, nullptr);
    lv_obj_add_style(hours, LV_OBJ_PART_MAIN, &time_style);
    sprintf(buff, "%02d", cvt_12_hour_clock(hh));
    lv_label_set_text(hours, buff);
    lv_obj_align(hours, view, LV_ALIGN_CENTER, -55, 0);

    //semicolon
    static lv_style_t dot_style;
    lv_style_init(&dot_style);
    lv_style_set_text_color(&dot_style, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_style_set_text_font(&dot_style, LV_STATE_DEFAULT, &liquidCrystal_nor_64);


    lv_obj_t *dot = lv_label_create(view, nullptr);
    lv_obj_add_style(dot, LV_OBJ_PART_MAIN, &dot_style);
    lv_label_set_text(dot, ":");
    lv_obj_align(dot, hours, LV_ALIGN_OUT_RIGHT_MID, 0, -10);

    //minute
    minute = lv_label_create(view, nullptr);
    lv_obj_add_style(minute, LV_OBJ_PART_MAIN, &time_style);
    sprintf(buff, "%02d", mm);
    lv_label_set_text(minute, buff);
    lv_obj_align(minute, hours, LV_ALIGN_OUT_RIGHT_MID, 15, 0);

    //Intermediate clock second digit
    static lv_style_t second_style;
    lv_style_init(&second_style);
    lv_style_set_text_color(&second_style, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_style_set_text_font(&second_style, LV_STATE_DEFAULT, &liquidCrystal_nor_32);

    second = lv_label_create(view, nullptr);
    lv_obj_add_style(second, LV_OBJ_PART_MAIN, &second_style);
    sprintf(buff, "%02d", ss);
    lv_label_set_text(second, buff);
    lv_obj_align(second, minute, LV_ALIGN_OUT_RIGHT_BOTTOM, 3, -10);

    //date
    static lv_style_t year_style;
    lv_style_init(&year_style);
    lv_style_set_text_color(&year_style, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_style_set_text_font(&year_style, LV_STATE_DEFAULT, &liquidCrystal_nor_24);


    year = lv_label_create(view, nullptr);
    lv_obj_add_style(year, LV_OBJ_PART_MAIN, &year_style);
    sprintf(buff, "%4d", yyear);
    lv_label_set_text(year, buff);
    lv_obj_align(year, view, LV_ALIGN_CENTER, 15, 55);

    month = lv_label_create(view, nullptr);
    lv_obj_add_style(month, LV_OBJ_PART_MAIN, &year_style);
    sprintf(buff, "%02d", mmonth );
    lv_label_set_text(month, buff);
    lv_obj_align(month, year, LV_ALIGN_OUT_LEFT_MID, -10, 0);

    day = lv_label_create(view, nullptr);
    lv_obj_add_style(day, LV_OBJ_PART_MAIN, &year_style);
    sprintf(buff, "%02d", dday);
    lv_label_set_text(day, buff);
    lv_obj_align(day, year, LV_ALIGN_OUT_LEFT_MID, -45, 0);

    // temperature
    static lv_style_t temp_style;
    lv_style_init(&temp_style);
    lv_style_set_text_color(&temp_style, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_style_set_text_font(&temp_style, LV_STATE_DEFAULT, &quostige_16);

    static lv_style_t bot_style;
    lv_style_init(&bot_style);
    lv_style_set_text_color(&bot_style, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_style_set_text_font(&bot_style, LV_STATE_DEFAULT, &exninja_22);

    str2 = lv_label_create(view, nullptr);
    lv_obj_add_style(str2, LV_OBJ_PART_MAIN, &bot_style);
    sprintf(buff, "%d", step_counter);
    lv_label_set_text(str2, buff);
    lv_obj_align(str2, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -10, -2);

    return view;
}

void LilyGo_Time(uint8_t fullUpdate) {

  get_time_in_tz(tzindex);
  // these is used for the alarm function:
  local_hour = hh;
  local_minute = mm;
  do {
    per = ttgo->power->getBattPercentage();
  } while(per > 100);
  temp = ttgo->power->getTemp();
  //Serial.printf("raw temp = %f, cooked = %.1f\n", temp, temp/6.0);
  if(fullUpdate) {
    setupGUI();
  }
//  sprintf(buff, "%2d *C", temp);
//  lv_label_set_text(temp_text, buff);
  //lv_label_set_text(am_pm, (!general_config.twelve_hr_clock) ? "  " : (hh < 12) ? "AM" : "PM");
  sprintf(buff, "%4d", yyear);
  lv_label_set_text(year, buff);
  sprintf(buff, "%02d", mmonth);
  lv_label_set_text(month, buff);
  sprintf(buff, "%02d", dday);
  lv_label_set_text(day, buff);
  sprintf(buff, "%02d", cvt_12_hour_clock(hh));
  lv_label_set_text(hours, buff);
  sprintf(buff, "%02d", mm);
  lv_label_set_text(minute, buff);
  sprintf(buff, "%02d", ss);
  lv_label_set_text(second, buff);
  sprintf(buff, "%3d%%", per);
  lv_label_set_text(batt, buff  );
  lv_label_set_text(indicator, (charge_cable_connected) ? "C " : ""  );
  lv_task_handler();
}
