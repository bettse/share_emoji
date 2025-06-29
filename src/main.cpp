/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "SD_MMC.h"
#include "lvgl_v8_port.h"
#include "pincfg.h"
#include <Arduino.h>
#include <esp_display_panel.hpp>
#include <lvgl.h>

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

void lv_example_png_1(void) {
  lv_obj_t *img;

  img = lv_img_create(lv_scr_act());
  // LVFileSystem::init('S');
  lv_img_set_src(img, "S:/Cowboy.png");
  lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_size(img, 300, 300);
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

static void fs_init(void) {
  if (!SD_MMC.setPins(SD_MMC_CLK_PIN, SD_MMC_CMD_PIN, SD_MMC_D0_PIN,
                      SD_MMC_D1_PIN, SD_MMC_D2_PIN, SD_MMC_D3_PIN)) {
    Serial.println("Failed to set SD card pins");
    return;
  }
  if (!SD_MMC.begin()) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD_MMC.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }
}

static void *sd_fs_open(lv_fs_drv_t *drv, const char *path, lv_fs_mode_t mode) {
  LV_UNUSED(drv);

  const char *flags = "";

  if (mode == LV_FS_MODE_WR)
    flags = FILE_WRITE;
  else if (mode == LV_FS_MODE_RD)
    flags = FILE_READ;
  else if (mode == (LV_FS_MODE_WR | LV_FS_MODE_RD))
    flags = FILE_WRITE;

  File f = SD_MMC.open(path, flags);
  if (!f) {
    Serial.println("Failed to open file!");
    return NULL;
  }

  File *lf = new File{f};

  // make sure at the beginning
  // fp->seek(0);

  return (void *)lf;
}

static lv_fs_res_t sd_fs_close(lv_fs_drv_t *drv, void *file_p) {
  LV_UNUSED(drv);

  File *fp = (File *)file_p;

  fp->close();

  delete (fp); // when close
  return LV_FS_RES_OK;
}

static lv_fs_res_t sd_fs_read(lv_fs_drv_t *drv, void *file_p, void *fileBuf,
                              uint32_t btr, uint32_t *br) {
  LV_UNUSED(drv);

  File *fp = (File *)file_p;

  *br = fp->read((uint8_t *)fileBuf, btr);

  return (int32_t)(*br) < 0 ? LV_FS_RES_UNKNOWN : LV_FS_RES_OK;
}

static lv_fs_res_t sd_fs_write(lv_fs_drv_t *drv, void *file_p, const void *buf,
                               uint32_t btw, uint32_t *bw) {
  LV_UNUSED(drv);

  File *fp = (File *)file_p;

  *bw = fp->write((const uint8_t *)buf, btw);

  return (int32_t)(*bw) < 0 ? LV_FS_RES_UNKNOWN : LV_FS_RES_OK;
}

static lv_fs_res_t sd_fs_seek(lv_fs_drv_t *drv, void *file_p, uint32_t pos,
                              lv_fs_whence_t whence) {
  LV_UNUSED(drv);

  File *fp = (File *)file_p;

  SeekMode mode;
  if (whence == LV_FS_SEEK_SET)
    mode = SeekSet;
  else if (whence == LV_FS_SEEK_CUR)
    mode = SeekCur;
  else if (whence == LV_FS_SEEK_END)
    mode = SeekEnd;

  fp->seek(pos, mode);

  return LV_FS_RES_OK;
}

static lv_fs_res_t sd_fs_tell(lv_fs_drv_t *drv, void *file_p, uint32_t *pos_p) {
  LV_UNUSED(drv);

  File *fp = (File *)file_p;

  *pos_p = fp->position();

  return LV_FS_RES_OK;
}

static void *sd_dir_open(lv_fs_drv_t *drv, const char *dirpath) {
  LV_UNUSED(drv);

  File root = SD_MMC.open(dirpath);
  if (!root) {
    Serial.println("Failed to open directory!");
    return NULL;
  }

  if (!root.isDirectory()) {
    Serial.println("Not a directory!");
    return NULL;
  }

  File *lroot = new File{root};

  return (void *)lroot;
}

static lv_fs_res_t sd_dir_read(lv_fs_drv_t *drv, void *dir_p, char *fn) {
  LV_UNUSED(drv);

  File *root = (File *)dir_p;
  fn[0] = '\0';

  File file = root->openNextFile();
  while (file) {
    if (strcmp(file.name(), ".") == 0 || strcmp(file.name(), "..") == 0) {
      continue;
    } else {
      if (file.isDirectory()) {
        Serial.print("  DIR : ");
        Serial.println(file.name());
        fn[0] = '/';
        strcpy(&fn[1], file.name());
      } else {
        Serial.print("  FILE: ");
        Serial.print(file.name());
        Serial.print("  SIZE: ");
        Serial.println(file.size());

        strcpy(fn, file.name());
      }
      break;
    }
    file = root->openNextFile();
  }

  return LV_FS_RES_OK;
}

static lv_fs_res_t sd_dir_close(lv_fs_drv_t *drv, void *dir_p) {
  LV_UNUSED(drv);

  File *root = (File *)dir_p;

  root->close();

  delete (root); // when close

  return LV_FS_RES_OK;
}

static void lv_port_sd_fs_init(void) {
  fs_init();

  static lv_fs_drv_t fs_drv;
  _lv_fs_init();
  lv_fs_drv_init(&fs_drv);

  fs_drv.letter = 'S';
  fs_drv.cache_size = 0;

  fs_drv.open_cb = sd_fs_open;
  fs_drv.close_cb = sd_fs_close;
  fs_drv.read_cb = sd_fs_read;
  fs_drv.write_cb = sd_fs_write;
  fs_drv.seek_cb = sd_fs_seek;
  fs_drv.tell_cb = sd_fs_tell;

  fs_drv.dir_close_cb = sd_dir_close;
  fs_drv.dir_open_cb = sd_dir_open;
  fs_drv.dir_read_cb = sd_dir_read;

  lv_fs_drv_register(&fs_drv);
}

void setup() {
  Serial.begin(115200);
  delay(1500); // Wait for Serial to be ready
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
