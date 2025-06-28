/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "lvgl_v8_port.h"
#include <Arduino.h>
#include <esp_display_panel.hpp>
#include <lvgl.h>

#include <WebServer.h>
#include <WiFi.h>

const char *ssid = "Beelight";
const char *password = "password";
const char *NORDIC_UART_SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
const char *NORDIC_UART_SERVICE_UUID_TX =
    "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";
const char *NORDIC_UART_SERVICE_UUID_RX =
    "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";

/**
/* To use the built-in examples and demos of LVGL uncomment the includes below
respectively.
 * You also need to copy `lvgl/examples` to `lvgl/src/examples`. Similarly for
the demos `lvgl/demos` to `lvgl/src/demos`.
 */
// #include <demos/lv_demos.h>
// #include <lv_ex/lv_examples.h>

using namespace esp_panel::drivers;
using namespace esp_panel::board;

/* ==================== DEfS ==================== */
void ui_init();

// Example: Create a label
static lv_obj_t *pointer;

// Add X label
static lv_obj_t *labelX;

// Add Y label
static lv_obj_t *labelY;

static lv_obj_t *labelRemoteMessage;


void setup() {
  Serial.begin(115200);

  Serial.println("Initializing board");
  Board *board = new Board();
  board->init();
  assert(board->begin());

  Serial.println("Initializing LVGL");
  lvgl_port_init(board->getLCD(), board->getTouch());

  Serial.println("Creating UI");
  /* Lock the mutex due to the LVGL APIs are not thread-safe */
  lvgl_port_lock(-1);

  ui_init();

  /* Release the mutex */
  lvgl_port_unlock();
}

void loop() {
  delay(1000);
}


void ui_init() {
  // Custom initialization code here
  Serial.println("Custom initialization");

  pointer = lv_label_create(lv_scr_act());
  lv_label_set_text(pointer, "+");
  lv_obj_set_style_text_color(pointer, lv_color_hex(0x00FF00), 0);
  lv_obj_set_style_text_font(pointer, &lv_font_montserrat_30, 0);
  lv_obj_align(pointer, LV_ALIGN_CENTER, 0, -20);

  labelX = lv_label_create(lv_scr_act());
  lv_label_set_text(labelX, "X=");
  lv_obj_set_style_text_font(labelX, &lv_font_montserrat_12, 0);
  lv_obj_align(labelX, LV_ALIGN_CENTER, 0, -12);

  labelY = lv_label_create(lv_scr_act());
  lv_label_set_text(labelY, "Y=");
  lv_obj_set_style_text_font(labelY, &lv_font_montserrat_12, 0);
  lv_obj_align(labelY, LV_ALIGN_CENTER, 0, 12);

  labelRemoteMessage = lv_label_create(lv_scr_act());
  lv_label_set_text(labelRemoteMessage, "placeholder");
  lv_obj_set_style_text_font(labelRemoteMessage, &lv_font_montserrat_20, 0);
  lv_obj_align(labelRemoteMessage, LV_ALIGN_CENTER, 0, -50);

}
