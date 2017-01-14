#pragma once
#include <pebble.h>

#define SETTINGS_KEY 1

// Define our settings struct
typedef struct ClaySettings {
  bool isAnimated;
	bool useVibrate;
}__attribute__((__packed__)) ClaySettings;

static void default_settings();
static void load_settings();
static void save_settings();
static void inbox_received_handler(DictionaryIterator *iter, void *context);
static void update_display();
static void handle_battery(BatteryChargeState charge_state);
static void handle_bluetooth(bool connected);
static void update_time();
static void tick_handler(struct tm *tick_time, TimeUnits units_changed);
static void timer_handler(void *context);
static void main_window_load(Window *window);
static void main_window_unload(Window *window);
static void init();
static void deinit();