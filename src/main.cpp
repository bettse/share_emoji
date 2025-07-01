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

#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
#include <DNSServer.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <WebSocketsClient.h>
#include <WiFi.h>
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager

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
lv_obj_t *busy = NULL;

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

WebSocketsClient webSocket;

bool post(String emoji) {
  const char update_url[] = "https://sockethook.ericbetts.dev/hook/JC3636W518";
  String json = String("{\"selected\":\"") + emoji + String("\"}");

  bool result = false;
  Serial.print("Connecting to server...");

  WiFiClientSecure *client = new WiFiClientSecure;
  if (client) {
    client->setInsecure();
    {
      // Add a scoping block for HTTPClient https to make sure it is destroyed
      // before WiFiClientSecure *client is
      HTTPClient https;

      Serial.print("[HTTPS] begin...\n");
      https.begin(*client, update_url);
      https.addHeader("Content-Type", "application/json");

      int httpCode = https.POST(json);

      Serial.print("[HTTPS] POST...\n");

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been
        // handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK ||
            httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = https.getString();
          Serial.println(payload);
          result = true;
        }
      } else {
        Serial.printf("[HTTPS] POST... failed, error: %s\n",
                      https.errorToString(httpCode).c_str());
      }

      https.end();
      if (busy) {
        lv_obj_del(busy);
        busy = NULL;
      }
    }

    delete client;
  } else {
    Serial.println("Unable to create client");
  }
  return result;
}

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

static void post_timer_cb(lv_timer_t *timer) {
  const char *src = (const char *)timer->user_data;

  post(src);
}

static void my_event_cb(lv_event_t *event) {
  Serial.println("Event callback triggered");
  // TODO: mutex to prevent multiple callbacks

  lv_obj_t *img = (lv_obj_t *)lv_event_get_user_data(event);

  if (busy) {
    lv_obj_del(busy);
    busy = NULL;
  }

  busy = lv_spinner_create(lv_scr_act(), 1000, 80);
  lv_obj_set_size(busy, 35, 35);
  lv_obj_align_to(busy, img, LV_ALIGN_CENTER, 0, 0);

  const char *src = (const char *)lv_img_get_src(img);
  lv_timer_t *timer = lv_timer_create_basic();
  timer->user_data = (void *)src;
  lv_timer_set_cb(timer, post_timer_cb);
  lv_timer_set_repeat_count(timer, 1);
}

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
  case WStype_DISCONNECTED:
    Serial.printf("[WSc] Disconnected!\n");
    break;
  case WStype_CONNECTED: {
    Serial.printf("[WSc] Connected to url: %s\n", payload);

    StaticJsonDocument<128> doc;
    doc["action"] = "subscribe";
    doc["key"] =
        "105a815ebf0233dc1e784fa47cf9824efc7643d3d7a1c9a4805160279b7d901c";
    String json;
    serializeJson(doc, json);
    webSocket.sendTXT(json);
  } break;
  case WStype_TEXT: {
    if (payload == NULL || length == 0) {
      return;
    }
    Serial.printf("[WSc] get text: %s\n", payload);
    // StaticJsonDocument<1536> doc;
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, payload, length);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }
    Serial.printf("[WSc] get json:\n");
    serializeJsonPretty(doc, Serial);
    Serial.println("");
    if (doc["selected"]) {
      selected = doc["selected"].as<String>();
      zoom_out_center();
    }
    break;
  }
  case WStype_BIN:
    Serial.printf("[WSc] get binary length: %u\n", length);
    // hexdump(payload, length);
    break;
  case WStype_ERROR:
  case WStype_FRAGMENT_TEXT_START:
  case WStype_FRAGMENT_BIN_START:
  case WStype_FRAGMENT:
  case WStype_FRAGMENT_FIN:
  case WStype_PING:
  case WStype_PONG:
    Serial.printf("[WSc] other event\n");
    break;
  }
}

void configModeCallback(WiFiManager *myWiFiManager) {
  lv_obj_t *label = lv_label_create(lv_scr_act());
  lv_label_set_text(label, "WiFi Config Mode");
  lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
  lv_obj_center(label);
  Serial.println("Entered config mode");
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

  lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLLABLE);

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
  // lv_obj_align(center, LV_ALIGN_CENTER, 0, 0);
  lv_obj_center(center);
  lv_img_set_antialias(center, true);
  lv_img_set_zoom(center, ZOOM_MIN);

  /* Release the mutex */
  lvgl_port_unlock();

  // TODO: put label on screen when no wifi connection

  WiFiManager wifiManager;
  // wifiManager.setBreakAfterConfig(true);
  // wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.autoConnect("AutoConnectAP");
  Serial.println("connected...yeey :)");

  webSocket.beginSSL("websocket.ericbetts.dev", 443);
  webSocket.setReconnectInterval(5000);
  webSocket.onEvent(webSocketEvent);
}

unsigned long previousMillis = 0;
unsigned long interval = 1000 * 60 * 5; // 5 minutes
void loop() {
  lv_timer_handler(); // let LVGL do its work

  webSocket.loop();
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis > interval) {
    previousMillis = currentMillis;
    // keepalive
    webSocket.sendTXT("{}");
  }
}
