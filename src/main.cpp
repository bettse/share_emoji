/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "fs.h"
#include "lvgl_v8_port.h"
#include "pincfg.h"
#include <Arduino.h>
#include <esp_display_panel.hpp>
#include <lvgl.h>

#include <WiFi.h>

#define SCREEN_WIDTH 360
#define SCREEN_HEIGHT 360

#define ZOOM_MAX (192)
#define ZOOM_MIN (256 / 16)
#define ZOOM_RING (256 / 8)

using namespace esp_panel::drivers;
using namespace esp_panel::board;

static String selected = "";
lv_obj_t *ring[8];
lv_obj_t *center;

const char *result[] = {"S:/Cowboy.png",
                        "S:/Slightly Smiling.png",
                        "S:/Sunglasses.png",
                        "S:/Surprised.png",
                        "S:/Thinking.png",
                        "S:/Thumbs Up.png",
                        "S:/Upside Down Smiling.png",
                        "S:/Waving Hand.png"};
int r45 = 140;
int xs[] = {
    0, r45 - (300 / 8),  SCREEN_WIDTH / 2 - (300 / 8),  r45 - (300 / 8),
    0, -r45 + (300 / 8), -SCREEN_WIDTH / 2 + (300 / 8), -r45 + (300 / 8)};
int ys[] = {
    SCREEN_HEIGHT / 2 - (300 / 8),  -r45 + (300 / 8), 0, r45 - (300 / 8),
    -SCREEN_HEIGHT / 2 + (300 / 8), r45 - (300 / 8),  0, -r45 + (300 / 8)};

void my_print(const char *buf) {
  Serial.printf(buf);
  Serial.flush();
}

static void anim_x_cb(void *var, int32_t v) {
  lv_obj_set_x((_lv_obj_t *)var, v);
}

static void anim_y_cb(void *var, int32_t v) {
  lv_obj_set_y((_lv_obj_t *)var, v);
}

static void anim_zoom_cb(void *var, int32_t v) {
  lv_img_set_zoom((_lv_obj_t *)var, v);
}

static void zoom_in_center() {
  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_var(&a, center);
  lv_anim_set_values(&a, lv_img_get_zoom(center), ZOOM_MAX);
  lv_anim_set_time(&a, 3000);
  lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
  lv_anim_set_exec_cb(&a, anim_zoom_cb);
  lv_anim_start(&a);
}

static void anim_zoom_out_ready_cb(lv_anim_t *a) {
  lv_img_set_src(center, selected.c_str());
  zoom_in_center();
}

static void zoom_out_center() {
  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_var(&a, center);
  lv_anim_set_values(&a, lv_img_get_zoom(center), ZOOM_MIN);
  lv_anim_set_time(&a, 3000);
  lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
  lv_anim_set_exec_cb(&a, anim_zoom_cb);
  lv_anim_set_ready_cb(&a, anim_zoom_out_ready_cb);
  lv_anim_start(&a);
}

static void my_event_cb(lv_event_t *event) {
  Serial.println("Event callback triggered");
  // TODO: mutex to prevent multiple animations

  lv_obj_t *img = (lv_obj_t *)lv_event_get_user_data(event);
  const char *src = (const char *)lv_img_get_src(img);
  selected = src ? String(src) : "";
  Serial.printf("Selected image: %s\n", selected.c_str());

  if (selected.length() > 0) {
    zoom_out_center();
  } else {
    lv_img_set_src(center, selected.c_str());
    zoom_in_center();
  }
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

  for (int i = 0; i < 8; i++) {
    lv_obj_t *img = lv_img_create(lv_scr_act());
    lv_img_set_src(img, result[i]);
    lv_obj_align(img, LV_ALIGN_CENTER, xs[i], ys[i]);
    lv_img_set_antialias(img, true);
    lv_img_set_zoom(img, ZOOM_RING);
    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(img, my_event_cb, LV_EVENT_CLICKED, img);
    ring[i] = img;
  }

  center = lv_img_create(lv_scr_act());
  lv_obj_align(center, LV_ALIGN_CENTER, 0, 0);
  lv_img_set_antialias(center, true);
  lv_img_set_zoom(center, ZOOM_MIN);

  /* Release the mutex */
  lvgl_port_unlock();
}

void loop() {
  lv_timer_handler(); // let LVGL do its work
  delay(1);
}
