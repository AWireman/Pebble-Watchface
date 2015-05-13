#include <pebble.h>
#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1	
  
static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_weather_layer;
//static GFont s_time_font;
static GFont s_weather_font;

static void update_time(){
	time_t temp = time(NULL);
	struct tm *tick_time = localtime(&temp);
	
	static char buffer[] = "00:00";
	
	if (clock_is_24h_style() == true ){
		strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
	}
	else{
		strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
	}
	
	text_layer_set_text(s_time_layer, buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
	// Get weather update every 30 minutes
if(tick_time->tm_min % 30 == 0) {
  // Begin dictionary
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  // Add a key-value pair
  dict_write_uint8(iter, 0, 0);

  // Send the message!
  app_message_outbox_send();
}

}

static void main_window_load(Window *window) {
	//time layer
	s_time_layer = text_layer_create(GRect(0, 55, 144, 50));
	text_layer_set_background_color(s_time_layer, GColorWhite);
	text_layer_set_text_color(s_time_layer, GColorBlack);
	text_layer_set_text(s_time_layer, "00:00");
	//s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DOS_45));
	//text_layer_set_font(s_time_layer, s_time_font);
	text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));//Sets window on top of main window
	
	//weather layer
	s_weather_layer = text_layer_create(GRect (0, 130, 144, 25));
	text_layer_set_background_color(s_weather_layer, GColorWhite);
	text_layer_set_text_color(s_weather_layer, GColorBlack);
	text_layer_set_text(s_weather_layer, "Loading...");
	text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
	s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DOS_20));
	text_layer_set_font(s_weather_layer, s_weather_font);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));//Sets window on top of main window
}
								
static void main_window_unload(Window *window) {
	//text layer
	text_layer_destroy(s_time_layer);
	//fonts_unload_custom_font(s_time_font);
	
	//weather layer
	text_layer_destroy(s_weather_layer);
	fonts_unload_custom_font(s_weather_font);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {


	
  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
		
			// Store incoming information
static char temperature_buffer[8];
static char conditions_buffer[32];
static char weather_layer_buffer[32];
		
    // Which key was received?
    switch(t->key) {
    case KEY_TEMPERATURE:
			snprintf(temperature_buffer, sizeof(temperature_buffer), "%dC", (int)t->value->int32);
      break;
    case KEY_CONDITIONS:
			snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }

		// Assemble full string and display
snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
text_layer_set_text(s_weather_layer, weather_layer_buffer);

    // Look for next item
    t = dict_read_next(iterator);
  }
}




static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

	
static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();
	
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
	update_time();
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
	
	//weather implementation
app_message_register_inbox_received(inbox_received_callback);
app_message_register_inbox_dropped(inbox_dropped_callback);
app_message_register_outbox_failed(outbox_failed_callback);
app_message_register_outbox_sent(outbox_sent_callback);

	
	// Open AppMessage
app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

}

static void deinit() {
  // Destroy Window
   window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
