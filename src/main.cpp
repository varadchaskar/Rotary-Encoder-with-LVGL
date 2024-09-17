#include <Arduino.h>
#include <FS.h>
#include <SPI.h>
#include <lvgl.h>
#include <TFT_eSPI.h>

// Pin definitions
#define TOUCH_CS 21
#define BUTTON_PIN_1 12
#define BUTTON_PIN_2 32 // Button to select highlighted items
#define BUZZER_PIN 13
#define CALIBRATION_FILE "/TouchCalData3"
#define REPEAT_CAL true
#define LVGL_REFRESH_TIME 20u
#define outputA 25
#define outputB 33

// Variables
int counter = 0;
int aState;
int aLastState;
int list_size = 5; // Example list size
int sublist_size = 4; // Example sublist size (including "Return" item)
int sublist_counter = 0; // Track the sublist counter
bool showing_sublist = false; // Track whether a sublist is being shown
unsigned long lastPressTime = 0;
const unsigned long debounceDelay = 300; // Adjust delay as necessary

TFT_eSPI tft = TFT_eSPI();
static const uint32_t screenWidth = 320;
static const uint32_t screenHeight = 240;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 10];
static lv_style_t style_default, style_selected;
lv_obj_t *label;
lv_obj_t *list;
lv_obj_t *list_items[5]; // Assuming we have 5 items in the list
lv_obj_t *sublist;
lv_obj_t *sublist_items[4]; // Assuming we have 3 sub-items + "Return"

// Function declarations
void touch_calibrate();
void lvgl_port_tp_read(lv_indev_drv_t *indev, lv_indev_data_t *data);
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
static void list_event_handler(lv_event_t *e);
static void sublist_event_handler(lv_event_t *e);
void lv_example_list(void);
void lv_create_sublist(int parent_item);
void lv_remove_sublist();
void handle_encoder_list();
void handle_encoder_sublist();
void handle_button_press();

void touch_calibrate() {
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  if (!SPIFFS.begin()) {
    Serial.println("Formatting file system");
    SPIFFS.format();
    SPIFFS.begin();
  }

  if (SPIFFS.exists(CALIBRATION_FILE)) {
    if (!REPEAT_CAL) {
      File f = SPIFFS.open(CALIBRATION_FILE, "r");
      if (f) {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL) {
    tft.setTouch(calData);
  } else {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");
    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}

void lvgl_port_tp_read(lv_indev_drv_t *indev, lv_indev_data_t *data) {
  uint16_t touchX, touchY;
  bool touched = tft.getTouch(&touchX, &touchY);

  if (!touched) {
    data->state = LV_INDEV_STATE_REL;
  } else {
    data->state = LV_INDEV_STATE_PR;
    data->point.x = touchX;
    data->point.y = touchY;
  }
}

void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t *)&color_p->full, w * h, true);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

static void list_event_handler(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_target(e);
  uint32_t index = (uint32_t)lv_event_get_user_data(e); // Get the list item index

  if (!showing_sublist) {
    lv_create_sublist(index);
  }
}

static void sublist_event_handler(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_target(e);
  uint32_t index = (uint32_t)lv_event_get_user_data(e); // Get the sublist item index

  if (index == sublist_size - 1) {
    lv_remove_sublist();
  }
}

void lv_example_list(void) {
  list = lv_list_create(lv_scr_act());
  lv_obj_set_size(list, 200, 150);
  lv_obj_align(list, LV_ALIGN_CENTER, 0, 0);

  for (int i = 0; i < list_size; i++) {
    char buf[10];
    snprintf(buf, sizeof(buf), "Item %d", i + 1);
    list_items[i] = lv_list_add_btn(list, NULL, buf);
    lv_obj_add_event_cb(list_items[i], list_event_handler, LV_EVENT_CLICKED, (void *)i);
  }

  lv_obj_add_style(list_items[0], &style_selected, 0);
}

void lv_create_sublist(int parent_item) {
  sublist = lv_list_create(lv_scr_act());
  lv_obj_set_size(sublist, 200, 150);
  lv_obj_align(sublist, LV_ALIGN_CENTER, 0, 0);

  for (int i = 0; i < sublist_size - 1; i++) {
    char buf[20];
    snprintf(buf, sizeof(buf), "Subitem %d-%d", parent_item + 1, i + 1);
    sublist_items[i] = lv_list_add_btn(sublist, NULL, buf);
    lv_obj_add_event_cb(sublist_items[i], sublist_event_handler, LV_EVENT_CLICKED, (void *)i);
  }

  sublist_items[sublist_size - 1] = lv_list_add_btn(sublist, NULL, "Return");
  lv_obj_add_event_cb(sublist_items[sublist_size - 1], sublist_event_handler, LV_EVENT_CLICKED, (void *)(sublist_size - 1));

  lv_obj_add_style(sublist_items[0], &style_selected, 0);
  showing_sublist = true;
}

void lv_remove_sublist() {
  if (sublist != NULL) {
    lv_obj_del(sublist);
    showing_sublist = false;
  }
}

void handle_encoder_list() {
  aState = digitalRead(outputA);
  if (aState != aLastState) {
    if (digitalRead(outputB) != aState) {
      counter++;
    } else {
      counter--;
    }

    if (counter < 0) counter = list_size - 1;
    if (counter >= list_size) counter = 0;

    for (int i = 0; i < list_size; i++) {
      if (i == counter) {
        lv_obj_add_style(list_items[i], &style_selected, 0);
      } else {
        lv_obj_remove_style(list_items[i], &style_selected, 0);
      }
    }

    aLastState = aState;
  }
}

void handle_encoder_sublist() {
  aState = digitalRead(outputA);
  if (aState != aLastState) {
    if (digitalRead(outputB) != aState) {
      sublist_counter++;
    } else {
      sublist_counter--;
    }

    if (sublist_counter < 0) sublist_counter = sublist_size - 1;
    if (sublist_counter >= sublist_size) sublist_counter = 0;

    for (int i = 0; i < sublist_size; i++) {
      if (i == sublist_counter) {
        lv_obj_add_style(sublist_items[i], &style_selected, 0);
      } else {
        lv_obj_remove_style(sublist_items[i], &style_selected, 0);
      }
    }

    aLastState = aState;
  }
}

void handle_button_press() {
  if (digitalRead(BUTTON_PIN_2) == LOW && millis() - lastPressTime > debounceDelay) {
    lastPressTime = millis();
    if (showing_sublist) {
      if (sublist_counter == sublist_size - 1) {
        lv_remove_sublist();
      }
    } else {
      lv_create_sublist(counter);
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(outputA, INPUT);
  pinMode(outputB, INPUT);
  pinMode(BUTTON_PIN_2, INPUT_PULLUP);

  aLastState = digitalRead(outputA);

  tft.begin();
  tft.setRotation(1);
  touch_calibrate();

  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * 10);

  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = lvgl_port_tp_read;
  lv_indev_drv_register(&indev_drv);

  lv_style_init(&style_selected);
  lv_style_set_bg_color(&style_selected, lv_color_hex(0xFF0000));

  lv_example_list();
}

void loop() {
  lv_timer_handler();
  delay(LVGL_REFRESH_TIME);

  if (showing_sublist) {
    handle_encoder_sublist();
  } else {
    handle_encoder_list();
  }

  handle_button_press();
}