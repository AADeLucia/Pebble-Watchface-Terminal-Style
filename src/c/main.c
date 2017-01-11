#include <pebble.h>

/*tester comment*/

static Window *s_main_window;
static GFont s_font;
static TextLayer *s_time_layer;
static TextLayer *s_battery_layer;
static TextLayer *s_connection_layer;
static TextLayer *s_text_layer;
static GBitmap *s_bitmap;
static BitmapLayer *s_bitmap_layer;
static GBitmapSequence *s_sequence;
static GBitmap *s_bitmap_cursor;
static BitmapLayer *s_bitmap_cursor_layer;

/***Handle Battery***/
static void handle_battery(BatteryChargeState charge_state) {
	static char battery_text[] = "100%";
	if(charge_state.is_charging) {
		snprintf(battery_text, sizeof(battery_text), "N/A");
	} else {
		snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);
	}
	
	static char s_buffer[35];
	snprintf(s_buffer, sizeof(s_buffer), "root@PC:/$ info\nbattery:   %s", battery_text);
	
	text_layer_set_text(s_battery_layer, s_buffer);
}

/***Handle Connection***/
static void handle_bluetooth(bool connected) {
	text_layer_set_text(s_connection_layer, connected ? "connected: yes" : "connected: no");
}

/***Handle Time***/
static void update_time() {
	//Get a tm structure
	time_t temp = time(NULL);
	struct tm *tick_time = localtime(&temp);
	
	//Write current hours and minutes into a buffer
	//Desired style: "Thu Mar 24 01:46"
	static char time_text[] = "day mon dd hh:mm EST YYYY";
	strftime(time_text, sizeof(time_text), clock_is_24h_style() ? 
					 "%a %b %e %R" : "%a %b %e %I:%M", tick_time);
	
	//Display time on the TextLayer
	static char s_buffer[50];
	snprintf(s_buffer, sizeof(s_buffer), "root@PC:/$ date\n%s",time_text);
	text_layer_set_text(s_time_layer, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	update_time();
}

/***Animation***/
#if defined (PBL_COLOR)
static void timer_handler(void *context) {
  uint32_t next_delay;

  // Advance to the next APNG frame, and get the delay for this frame
  if(gbitmap_sequence_update_bitmap_next_frame(s_sequence, s_bitmap_cursor, &next_delay)) {
    // Set the new frame into the BitmapLayer
    bitmap_layer_set_bitmap(s_bitmap_cursor_layer, s_bitmap_cursor);
    layer_mark_dirty(bitmap_layer_get_layer(s_bitmap_cursor_layer));

    // Timer for that frame's delay
    app_timer_register(next_delay, timer_handler, NULL);
  }
}
#endif

/***Handle Window***/
static void main_window_load(Window *window) {
	//Get information about the window
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);
	
	//Use platform specific background image
	s_bitmap = gbitmap_create_with_resource(
		PBL_IF_BW_ELSE(RESOURCE_ID_IMAGE_BACKGROUND_BW, RESOURCE_ID_IMAGE_BACKGROUND_COLOR));
	s_bitmap_layer = bitmap_layer_create(GRect(0, 0, 144, 168)); //size of image
	bitmap_layer_set_compositing_mode(s_bitmap_layer, GCompOpSet);
	bitmap_layer_set_bitmap(s_bitmap_layer, s_bitmap);
	window_set_background_color(s_main_window, GColorBlack);
	layer_add_child(window_layer, bitmap_layer_get_layer(s_bitmap_layer));
	
	//Add custom font
	s_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MONACO_14));	
		
	//Time layer
	s_time_layer = text_layer_create(GRect(5, 30, bounds.size.w, 40));
	text_layer_set_background_color(s_time_layer, GColorClear);
	text_layer_set_text_color(s_time_layer, GColorWhite);
	text_layer_set_font(s_time_layer, s_font);
	layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
	
	//Battery layer
	s_battery_layer = text_layer_create(GRect(5, 70, bounds.size.w, 50));
	text_layer_set_background_color(s_battery_layer, GColorClear);
	text_layer_set_text_color(s_battery_layer, GColorWhite);
	text_layer_set_font(s_battery_layer, s_font);
	layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));
	
	//Connection layer
	s_connection_layer = text_layer_create(GRect(5, 100, bounds.size.w, 20));
	text_layer_set_background_color(s_connection_layer, GColorClear);
	text_layer_set_text_color(s_connection_layer, GColorWhite);
	text_layer_set_font(s_connection_layer, s_font);
	handle_bluetooth(connection_service_peek_pebble_app_connection());
	layer_add_child(window_layer, text_layer_get_layer(s_connection_layer));
	
	//Other text layer
	s_text_layer = text_layer_create(GRect(5, 125, bounds.size.w, 20));
	text_layer_set_background_color(s_text_layer, GColorClear);
	text_layer_set_text_color(s_text_layer, GColorWhite);
	text_layer_set_font(s_text_layer, s_font);
	text_layer_set_text(s_text_layer, "root@PC:/$");
	layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
	
	//Platform-specific cursor animation
	//Stationary cursor for Aplite
	#if defined (PBL_BW)
	s_bitmap_cursor = gbitmap_create_with_resource(RESOURCE_ID_STATIC_CURSOR);
	//Blinking cursor for Basalt
	#elif defined (PBL_COLOR)
	s_sequence = gbitmap_sequence_create_with_resource(RESOURCE_ID_BLINKING_CURSOR);
	GSize frame_size = gbitmap_sequence_get_bitmap_size(s_sequence);
	s_bitmap_cursor = gbitmap_create_blank(frame_size, GBitmapFormat8Bit);
	//Start the animation
	uint32_t first_delay_ms = 1000;
	app_timer_register(first_delay_ms, timer_handler, NULL);
	#endif
	
	s_bitmap_cursor_layer = bitmap_layer_create(GRect(90, 125, 10, 15));
	bitmap_layer_set_compositing_mode(s_bitmap_cursor_layer, GCompOpSet);
	bitmap_layer_set_bitmap(s_bitmap_cursor_layer, s_bitmap_cursor);
	layer_add_child(window_layer, bitmap_layer_get_layer(s_bitmap_cursor_layer));
}

static void main_window_unload(Window *window) {
	//Destroy elements
	fonts_unload_custom_font(s_font);
	text_layer_destroy(s_battery_layer);
	text_layer_destroy(s_connection_layer);
	text_layer_destroy(s_time_layer);
	gbitmap_destroy(s_bitmap);
	bitmap_layer_destroy(s_bitmap_layer);
	gbitmap_destroy(s_bitmap_cursor);
	bitmap_layer_destroy(s_bitmap_cursor_layer);
	gbitmap_sequence_destroy(s_sequence);
}

static void init() {
	//Create main window
	s_main_window = window_create();
	
	//Set window handlers
	window_set_window_handlers(s_main_window, (WindowHandlers) {
		.load = main_window_load,
		.unload = main_window_unload
	});
	
	//Show window on the watch, with animated = true
	window_stack_push(s_main_window, true);
	
	//Display time at the start
	update_time();
	
	//Register with services
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
	battery_state_service_subscribe(handle_battery);
	connection_service_subscribe((ConnectionHandlers) {
		.pebble_app_connection_handler = handle_bluetooth
	});
	handle_battery(battery_state_service_peek());
}

static void deinit() {
	//Destroy window
	window_destroy(s_main_window);
	
	//Unregister with services
	tick_timer_service_unsubscribe();
	battery_state_service_unsubscribe();
	connection_service_unsubscribe();
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}