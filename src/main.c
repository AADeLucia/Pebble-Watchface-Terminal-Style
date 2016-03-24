#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static GFont s_time_font;

#define MAX_BUFFER_SIZE 50

static void update_time() {
	//Get a tm structure
	time_t temp = time(NULL);
	struct tm *tick_time = localtime(&temp);
	
	//Write current hours and minutes into a buffer
	//Desired style: "Thu Mar 24 01:46:37 UTC 2016"
	static char s_buffer[MAX_BUFFER_SIZE];
	strftime(s_buffer, MAX_BUFFER_SIZE, clock_is_24h_style() ? 
					 "root@PC:~$ date\n%a %b %e\n%R %Z %Y" : "root@PC:~$ date\n%a %b %e\n%I:%M %Z %Y", tick_time);
	
	//Display time on the TextLayer
	text_layer_set_text(s_time_layer, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	update_time();
}

static void main_window_load(Window *window) {
	//Get information about the window
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);
	
	//Create TextLayer
	s_time_layer = text_layer_create(GRect(10, PBL_IF_BW_ELSE(58, 52), bounds.size.w, 50));
	
	//Create GFont
	//s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_BITWISE_15));
	s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MONACO_16));
	
	//Customize the layout
	window_set_background_color(s_main_window, GColorBlack);
	text_layer_set_background_color(s_time_layer, GColorBlack);
	text_layer_set_text_color(s_time_layer, GColorWhite);
	text_layer_set_font(s_time_layer, s_time_font);
	text_layer_set_text_alignment(s_time_layer, GTextAlignmentLeft);
	
	//Add the text layer to the Window's root layer
	layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
}

static void main_window_unload(Window *window) {
	//Destroy elements
	text_layer_destroy(s_time_layer);
	fonts_unload_custom_font(s_time_font);
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
	
	//Register with TickTimerService
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
	//Destroy window
	window_destroy(s_main_window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}