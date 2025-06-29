/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "lvgl_v8_port.h"
#include "pincfg.h"
#include <Arduino.h>
#include <esp_display_panel.hpp>
#include <lvgl.h>
#include "fs.h"

#include <WiFi.h>

using namespace esp_panel::drivers;
using namespace esp_panel::board;

// Example: Create a label
static lv_obj_t *pointer;

// Add X label
static lv_obj_t *labelX;

// Add Y label
static lv_obj_t *labelY;

static lv_obj_t *labelRemoteMessage;

void my_print(const char *buf) {
  Serial.printf(buf);
  Serial.flush();
}

static void anim_x_cb(void *var, int32_t v) {
  lv_obj_set_x((_lv_obj_t *)var, v);
}

static void anim_size_cb(void *var, int32_t v) {
  lv_obj_set_size((_lv_obj_t *)var, v, v);
}

static void anim_zoom_cb(void *var, int32_t v) {
  lv_img_set_zoom((_lv_obj_t *)var, v);
}

void lv_example_png_1(void) {
  lv_obj_t *img;

  img = lv_img_create(lv_scr_act());
  lv_img_set_src(img, "S:/Sunglasses.png");

  lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
  //lv_obj_set_pos(img, 0, 0);
  //lv_obj_set_size(img, 200, 200); //lv_pct(100), lv_pct(100));

  lv_img_set_antialias(img, true);
  // lv_img_set_zoom(img, 256 / 2);
  // lv_img_set_pivot(img, 0, 0);  //To zoom from the left top corner

  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_var(&a, img);
  lv_anim_set_values(&a, LV_IMG_ZOOM_NONE, 256 / 8); // Zoom in and out

  lv_anim_set_time(&a, 5000);
  lv_anim_set_playback_delay(&a, 1000);
  lv_anim_set_playback_time(&a, 5000);
  lv_anim_set_repeat_delay(&a, 5000);
  lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
  lv_anim_set_path_cb(&a, lv_anim_path_linear);
  lv_anim_set_exec_cb(&a, anim_zoom_cb);
  lv_anim_start(&a);
}

void lv_example_anim_2(void) {
  lv_obj_t *obj = lv_obj_create(lv_scr_act());
  lv_obj_set_style_bg_color(obj, lv_palette_main(LV_PALETTE_RED), 0);
  lv_obj_set_style_radius(obj, LV_RADIUS_CIRCLE, 0);

  lv_obj_align(obj, LV_ALIGN_LEFT_MID, 10, 0);

  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_var(&a, obj);
  lv_anim_set_values(&a, 10, 50);

  lv_anim_set_time(&a, 1000);
  lv_anim_set_playback_delay(&a, 100);
  lv_anim_set_playback_time(&a, 300);
  lv_anim_set_repeat_delay(&a, 500);
  lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
  lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);

  lv_anim_set_exec_cb(&a, anim_size_cb);
  lv_anim_start(&a);
  lv_anim_set_exec_cb(&a, anim_x_cb);

  lv_anim_set_values(&a, 0, 180);
  lv_anim_start(&a);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Setup");

  lv_log_register_print_cb(my_print);

  Serial.println("Initializing board");
  Board *board = new Board();
  board->init();
  assert(board->begin());

  Serial.println("Initializing LVGL");
  lvgl_port_init(board->getLCD(), board->getTouch());

  // Had to move this lower to make it work
  lv_port_sd_fs_init();

  Serial.println("Creating UI");
  /* Lock the mutex due to the LVGL APIs are not thread-safe */
  lvgl_port_lock(-1);

  // lv_example_anim_2();
  lv_example_png_1();

  /* Release the mutex */
  lvgl_port_unlock();
}

unsigned long interval = 1000; // Interval in milliseconds
unsigned long previousMillis = 0;
void loop() {
  lv_timer_handler(); // let LVGL do its work
  // delay(5);

  /*
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis > interval) {
    previousMillis = currentMillis;
    Serial.println("beep");
  }
  */
}
