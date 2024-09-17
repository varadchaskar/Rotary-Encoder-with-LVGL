/*
 * Project: TFT Touch Display with Rotary Encoder and Sublist Menu
 * Author: Varad Chaskar
 * Date: 16-Sep-2024
 *
 * Description:
 * This project uses a TFT display with touch capabilities to create a list-based menu
 * system. A rotary encoder is used to navigate between menu items, and a button selects
 * the highlighted item. When a list item is selected, a sublist is shown with additional 
 * items. The project also utilizes LittlevGL (lvgl) for creating and managing the graphical
 * user interface (GUI). A button press is used to navigate into submenus, and the encoder 
 * helps in selecting items from both the list and sublist.
 *
 * Required Libraries:
 * 1. Arduino.h (for core functions)
 * 2. FS.h (for file system functions)
 * 3. SPI.h (for communication with the TFT display)
 * 4. lvgl.h (for managing the GUI)
 * 5. TFT_eSPI.h (for controlling the TFT display)
 */

#include <Arduino.h>
#include <FS.h>
#include <SPI.h>
#include <lvgl.h>
#include <TFT_eSPI.h>

// Pin definitions
#define TOUCH_CS 21           // Pin for touch screen chip select
#define BUTTON_PIN_1 12       // Pin for button 1 (not used in this code)
#define BUTTON_PIN_2 32       // Pin for button to select highlighted items
#define BUZZER_PIN 13         // Pin for buzzer (not used in this code)
#define CALIBRATION_FILE "/TouchCalData3" // File to store touch screen calibration data
#define REPEAT_CAL true       // Force calibration on every start if set to true
#define LVGL_REFRESH_TIME 20u // Refresh rate for the LittlevGL GUI in milliseconds
#define outputA 25            // Pin A for rotary encoder
#define outputB 33            // Pin B for rotary encoder

// Variables for rotary encoder
int counter = 0;              // Tracks current position in the list
int aState;                   // Current state of encoder pin A
int aLastState;               // Previous state of encoder pin A
int list_size = 5;            // Total number of items in the main list
int sublist_size = 4;         // Total number of items in the sublist (including "Return")
int sublist_counter = 0;      // Tracks the current position in the sublist
bool showing_sublist = false; // Flag to indicate if a sublist is being shown
unsigned long lastPressTime = 0;          // Time of the last button press
const unsigned long debounceDelay = 300;  // Debounce delay for the button in milliseconds

// TFT display and lvgl setup
TFT_eSPI tft = TFT_eSPI();                  // Create an instance of the TFT_eSPI class for the display
static const uint32_t screenWidth = 320;     // Width of the screen
static const uint32_t screenHeight = 240;    // Height of the screen
static lv_disp_draw_buf_t draw_buf;          // Buffer for drawing on the screen
static lv_color_t buf[screenWidth * 10];     // Buffer size for display
static lv_style_t style_default, style_selected; // GUI styles for default and selected items
lv_obj_t *label;                            // Pointer for the label widget
lv_obj_t *list;                             // Pointer for the main list widget
lv_obj_t *list_items[5];                    // Array to store list items (5 in total)
lv_obj_t *sublist;                          // Pointer for the sublist widget
lv_obj_t *sublist_items[4];                 // Array to store sublist items (4 in total)

// Function declarations
void touch_calibrate();                     // Function to calibrate the touch screen
void lvgl_port_tp_read(lv_indev_drv_t *indev, lv_indev_data_t *data); // Function to read touch screen input
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p); // Function to update display with lvgl buffer
static void list_event_handler(lv_event_t *e); // Function to handle events in the main list
static void sublist_event_handler(lv_event_t *e); // Function to handle events in the sublist
void lv_example_list(void);                 // Function to create the main list
void lv_create_sublist(int parent_item);    // Function to create the sublist based on the selected parent item
void lv_remove_sublist();                   // Function to remove the sublist from the screen
void handle_encoder_list();                 // Function to handle rotary encoder navigation for the main list
void handle_encoder_sublist();              // Function to handle rotary encoder navigation for the sublist
void handle_button_press();                 // Function to handle the button press for selecting items

// Calibrate the touch screen and store calibration data in SPIFFS
void touch_calibrate() {
  uint16_t calData[5];      // Array to hold calibration data
  uint8_t calDataOK = 0;    // Flag to check if calibration data exists

  // Start SPIFFS (SPI Flash File System)
  if (!SPIFFS.begin()) {
    Serial.println("Formatting file system");
    SPIFFS.format();        // Format if there's an error
    SPIFFS.begin();         // Restart SPIFFS
  }

  // Check if calibration data already exists in the file
  if (SPIFFS.exists(CALIBRATION_FILE)) {
    if (!REPEAT_CAL) {      // Skip calibration if not forced
      File f = SPIFFS.open(CALIBRATION_FILE, "r");  // Open calibration file
      if (f) {
        if (f.readBytes((char *)calData, 14) == 14) // Read the calibration data
          calDataOK = 1;     // Data is OK
        f.close();           // Close the file
      }
    }
  }

  // If valid calibration data exists and repeat calibration is false, use the existing data
  if (calDataOK && !REPEAT_CAL) {
    tft.setTouch(calData);   // Set touch calibration
  } else {
    tft.fillScreen(TFT_BLACK);   // Fill the screen with black before calibration
    tft.setCursor(20, 0);        // Set the text cursor
    tft.setTextFont(2);          // Set font size
    tft.setTextSize(1);          // Set text size
    tft.setTextColor(TFT_WHITE, TFT_BLACK); // Set text color

    tft.println("Touch corners as indicated"); // Instruction message for calibration
    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15); // Start calibration

    // Save calibration data to file
    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calData, 14); // Write the calibration data
      f.close();                                   // Close the file
    }
  }
}

// Function to read the touch screen input for lvgl
void lvgl_port_tp_read(lv_indev_drv_t *indev, lv_indev_data_t *data) {
  uint16_t touchX, touchY;       // Variables to hold touch coordinates
  bool touched = tft.getTouch(&touchX, &touchY); // Check if the screen is touched

  if (!touched) {
    data->state = LV_INDEV_STATE_REL; // If not touched, set input state to released
  } else {
    data->state = LV_INDEV_STATE_PR;  // If touched, set input state to pressed
    data->point.x = touchX;           // Set X coordinate
    data->point.y = touchY;           // Set Y coordinate
  }
}

// Function to flush the lvgl display buffer to the TFT screen
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);  // Width of the area to be flushed
  uint32_t h = (area->y2 - area->y1 + 1);  // Height of the area to be flushed

  tft.startWrite();        // Start writing to the TFT
  tft.setAddrWindow(area->x1, area->y1, w, h);  // Set the area to be updated
  tft.pushColors((uint16_t *)&color_p->full, w * h, true); // Push the colors to the screen
  tft.endWrite();          // End writing

  lv_disp_flush_ready(disp); // Tell lvgl that flushing is done
}

// Event handler for when a main list item is clicked
static void list_event_handler(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_target(e);  // Get the clicked list item object
  uint32_t index = (uint32_t)lv_event_get_user_data(e); // Get the index of the clicked item

  if (!showing_sublist) { // If sublist is not already showing
    lv_create_sublist(index); // Create sublist for the selected item
  }
}

// Event handler for when a sublist item is clicked
static void sublist_event_handler(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_target(e);  // Get the clicked sublist item object
  uint32_t index = (uint32_t)lv_event_get_user_data(e); // Get the index of the clicked sublist item

  if (index == 0) {         // If "Return" is selected
    lv_remove_sublist();     // Remove the sublist from the screen
  }
}

// Function to create the main list with items
void lv_example_list(void) {
  list = lv_list_create(lv_scr_act());     // Create a list object on the active screen

  // Create 5 items in the list and add them to the screen
  for (int i = 0; i < list_size; i++) {
    list_items[i] = lv_list_add_btn(list, NULL, "Item"); // Add each item to the list
    lv_obj_add_event_cb(list_items[i], list_event_handler, LV_EVENT_CLICKED, (void *)i); // Add event callback for item click
  }

  lv_obj_align(list, LV_ALIGN_CENTER, 0, 0);  // Center the list on the screen
}

// Function to create a sublist based on the selected parent item
void lv_create_sublist(int parent_item) {
  showing_sublist = true;      // Set flag to show sublist

  sublist = lv_list_create(lv_scr_act());    // Create a sublist object on the active screen

  // Create 4 sublist items (1st item is "Return" to go back to main list)
  sublist_items[0] = lv_list_add_btn(sublist, NULL, "Return");   // Add "Return" button to the sublist
  lv_obj_add_event_cb(sublist_items[0], sublist_event_handler, LV_EVENT_CLICKED, (void *)0); // Add event callback for "Return"
  
  for (int i = 1; i < sublist_size; i++) {
    sublist_items[i] = lv_list_add_btn(sublist, NULL, "SubItem"); // Add subitems to the sublist
    lv_obj_add_event_cb(sublist_items[i], sublist_event_handler, LV_EVENT_CLICKED, (void *)i); // Add event callback for subitem click
  }

  lv_obj_align(sublist, LV_ALIGN_CENTER, 0, 0); // Center the sublist on the screen
}

// Function to remove the sublist and return to the main list
void lv_remove_sublist() {
  lv_obj_del(sublist);      // Delete the sublist object from the screen
  showing_sublist = false;  // Reset flag to indicate sublist is no longer showing
}

// Function to handle the rotary encoder navigation for the main list
void handle_encoder_list() {
  aState = digitalRead(outputA);   // Read the state of encoder pin A

  // If the state has changed (indicating rotation)
  if (aState != aLastState) {
    if (digitalRead(outputB) != aState) {  // If pin B state differs from A
      counter++;       // Clockwise rotation (increment counter)
    } else {
      counter--;       // Counterclockwise rotation (decrement counter)
    }

    // Ensure the counter stays within valid bounds (0 to list_size-1)
    if (counter >= list_size) counter = 0;
    if (counter < 0) counter = list_size - 1;

    // Update the selected item style
    for (int i = 0; i < list_size; i++) {
      if (i == counter) {
        lv_obj_add_style(list_items[i], &style_selected, 0); // Apply selected style to the current item
      } else {
        lv_obj_remove_style(list_items[i], &style_selected, 0); // Remove selected style from other items
      }
    }
  }
  aLastState = aState;     // Save the last state of encoder pin A
}

// Function to handle the rotary encoder navigation for the sublist
void handle_encoder_sublist() {
  aState = digitalRead(outputA);   // Read the state of encoder pin A

  // If the state has changed (indicating rotation)
  if (aState != aLastState) {
    if (digitalRead(outputB) != aState) {  // If pin B state differs from A
      sublist_counter++;    // Clockwise rotation (increment sublist counter)
    } else {
      sublist_counter--;    // Counterclockwise rotation (decrement sublist counter)
    }

    // Ensure the sublist counter stays within valid bounds (0 to sublist_size-1)
    if (sublist_counter >= sublist_size) sublist_counter = 0;
    if (sublist_counter < 0) sublist_counter = sublist_size - 1;

    // Update the selected sublist item style
    for (int i = 0; i < sublist_size; i++) {
      if (i == sublist_counter) {
        lv_obj_add_style(sublist_items[i], &style_selected, 0); // Apply selected style to current sublist item
      } else {
        lv_obj_remove_style(sublist_items[i], &style_selected, 0); // Remove selected style from other sublist items
      }
    }
  }
  aLastState = aState;     // Save the last state of encoder pin A
}

// Function to handle the button press to select highlighted items
void handle_button_press() {
  if (digitalRead(BUTTON_PIN_2) == LOW) {   // If button is pressed
    unsigned long current_time = millis();  // Get the current time

    // Check for debounce (button press should only trigger after debounceDelay)
    if (current_time - lastPressTime > debounceDelay) {
      lastPressTime = current_time;  // Update last press time

      if (showing_sublist) {
        lv_event_send(sublist_items[sublist_counter], LV_EVENT_CLICKED, NULL); // Trigger click event for selected sublist item
      } else {
        lv_event_send(list_items[counter], LV_EVENT_CLICKED, NULL); // Trigger click event for selected main list item
      }
    }
  }
}

// Main setup function (runs once)
void setup() {
  Serial.begin(115200);     // Initialize serial communication for debugging

  // Set pin modes for the rotary encoder and button
  pinMode(outputA, INPUT_PULLUP);       // Set pin A as input
  pinMode(outputB, INPUT_PULLUP);       // Set pin B as input
  pinMode(BUTTON_PIN_2, INPUT_PULLUP); // Set button pin as input with pull-up resistor

  aLastState = digitalRead(outputA);  // Initialize the last state of encoder pin A

  tft.begin();              // Initialize the TFT display
  tft.setRotation(1);       // Set the display rotation (landscape)
  touch_calibrate();        // Calibrate the touch screen

  lv_init();                // Initialize LittlevGL (lvgl) for GUI management
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * 10);  // Initialize lvgl draw buffer

  // Initialize the lvgl display driver
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = screenWidth;     // Set horizontal resolution
  disp_drv.ver_res = screenHeight;    // Set vertical resolution
  disp_drv.flush_cb = my_disp_flush;  // Set flush callback to update the display
  disp_drv.draw_buf = &draw_buf;      // Set draw buffer
  lv_disp_drv_register(&disp_drv);    // Register the display driver with lvgl

  // Initialize the lvgl touch input driver
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;   // Set input type as pointer (touchscreen)
  indev_drv.read_cb = lvgl_port_tp_read;    // Set read callback for touch input
  lv_indev_drv_register(&indev_drv);        // Register the input driver with lvgl

  // Initialize lvgl styles for selected and default items
  lv_style_init(&style_selected);
  lv_style_set_bg_color(&style_selected, lv_color_hex(0xFF0000)); // Set selected style background color (red)

  // Create the main list on the screen
  lv_example_list();
}

// Main loop function (runs repeatedly)
void loop() {
  lv_timer_handler();        // Handle lvgl tasks (GUI refresh)
  delay(LVGL_REFRESH_TIME);  // Delay to control refresh rate

  if (showing_sublist) {     // If a sublist is being shown
    handle_encoder_sublist();  // Handle rotary encoder for sublist navigation
  } else {
    handle_encoder_list();     // Handle rotary encoder for main list navigation
  }

  handle_button_press();     // Handle button press for item selection
}
