
#include "main_window.h"

static Window *s_window;
static TextLayer *s_digits[NUM_CHARS];
static Layer *s_bt_layer;
static GBitmap *s_bt_bitmap;

static Layer *s_date_layer;

static char s_time_buffer[NUM_CHARS + 1];       // + NULL char
static char s_date_buffer[DATE_BUFFER_SIZE];
static int s_digit_states_now[NUM_CHARS - 1];   // No colon in this array
static int s_digit_states_prev[NUM_CHARS - 1];

/******************************** Digit logic *********************************/

static void update_digit_values(struct tm *tick_time) {
  strftime(s_time_buffer, sizeof(s_time_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);

  s_digit_states_now[3] = s_time_buffer[4] - '0'; // Convert to int
  s_digit_states_now[2] = s_time_buffer[3] - '0';
  s_digit_states_now[1] = s_time_buffer[1] - '0';
  s_digit_states_now[0] = s_time_buffer[0] - '0';

  strftime(s_date_buffer, sizeof(s_date_buffer), "%Y-%m-%d", tick_time); // "%a %d"
  
  // set date buffer to "2222-22-22" for testing
  // s_date_buffer[0] = '2';
  // s_date_buffer[1] = '2';
  // s_date_buffer[2] = '2';
  // s_date_buffer[3] = '2';
  // s_date_buffer[4] = '-';
  // s_date_buffer[5] = '2';
  // s_date_buffer[6] = '2';
  // s_date_buffer[7] = '-';
  // s_date_buffer[8] = '2';
  // s_date_buffer[9] = '2';
  // s_date_buffer[10] = '\0';


}

static void show_digit_values() {
  static char s_chars[5][2] = {"1", "2", ":", "3", "4"};
  for(int i = 0; i < NUM_CHARS; i++) {
    if(i != 2) {
      s_chars[i][0] = s_time_buffer[i];
      text_layer_set_text(s_digits[i], DEBUG ? "0" : s_chars[i]);
    } else {
      text_layer_set_text(s_digits[i], ":");
    }
  }
}

/*********************************** Window ***********************************/

static void bt_handler(bool connected) {
  if(connected) {
    // layer_set_hidden(s_bt_layer, true);
    layer_set_hidden(s_bt_layer, false);
  } else {
    vibes_double_pulse();
    layer_set_hidden(s_bt_layer, false);
  }
}

static void date_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, data_get_foreground_color());
  // Set the line color
  graphics_context_set_stroke_color(ctx, data_get_foreground_color());

  // Set the stroke width (must be an odd integer value)
  graphics_context_set_stroke_width(ctx, 1);

  // Disable antialiasing (enabled by default where available)
  graphics_context_set_antialiased(ctx, true);

  // get the bounds of the layer
  GRect bounds = layer_get_bounds(layer);
  
  // convert s_date_buffer to string
  font_draw_string(s_date_buffer, bounds.origin.x, bounds.origin.y, 0.65, ctx);
}

static void bt_update_proc(Layer *layer, GContext *ctx) {
  if(!data_get_boolean_setting(DataKeyBTIndicator)) {
    return;
  }

  // Draw it white
  graphics_context_set_compositing_mode(ctx, GCompOpSet);
  graphics_draw_bitmap_in_rect(ctx, s_bt_bitmap, gbitmap_get_bounds(s_bt_bitmap));

  // Swap to FG color
  GBitmap *fb = graphics_capture_frame_buffer(ctx);
  universal_fb_swap_colors(fb, layer_get_frame(layer), GColorWhite, data_get_foreground_color());
  graphics_release_frame_buffer(ctx, fb);
}

static void tick_handler(struct tm* tick_time, TimeUnits changed) {
  update_digit_values(tick_time);
  show_digit_values();  
}

// MAIN WINDOW
static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_digits[0] = text_layer_create(GRect(HOURS_TENS_X_OFFSET, Y_OFFSET, DIGIT_SIZE.w, DIGIT_SIZE.h));
  s_digits[1] = text_layer_create(GRect(HOURS_UNITS_X_OFFSET, Y_OFFSET, DIGIT_SIZE.w, DIGIT_SIZE.h));
  s_digits[2] = text_layer_create(GRect(COLON_X_OFFSET, Y_OFFSET, DIGIT_SIZE.w, DIGIT_SIZE.h));
  s_digits[3] = text_layer_create(GRect(MINS_TENS_X_OFFSET, Y_OFFSET, DIGIT_SIZE.w, DIGIT_SIZE.h));
  s_digits[4] = text_layer_create(GRect(MINS_UNITS_X_OFFSET, Y_OFFSET, DIGIT_SIZE.w, DIGIT_SIZE.h));

  for(int i = 0; i < NUM_CHARS; i++) {
    text_layer_set_background_color(s_digits[i], GColorClear);
    text_layer_set_text_alignment(s_digits[i], GTextAlignmentRight); // GTextAlignmentCenter
    text_layer_set_font(s_digits[i], fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_48)));
    layer_add_child(window_layer, text_layer_get_layer(s_digits[i]));
  }

  // Date layer
  s_date_layer = layer_create(GRect(DATE_X_OFFSET, DATE_Y_OFFSET, bounds.size.w - DATE_X_OFFSET, DATE_HEIGHT));
  layer_set_update_proc(s_date_layer, date_update_proc);
  layer_add_child(window_layer, s_date_layer);

  s_bt_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BT_WHITE);

  GRect bitmap_bounds = gbitmap_get_bounds(s_bt_bitmap);
  s_bt_layer = layer_create(GRect(
    (bounds.size.w - bitmap_bounds.size.w) / 2,
    BT_Y_OFFSET, bitmap_bounds.size.w, bitmap_bounds.size.h));
  layer_set_update_proc(s_bt_layer, bt_update_proc);
  layer_add_child(window_layer, s_bt_layer);
}

static void window_unload(Window *window) {
  for(int i = 0; i < NUM_CHARS; i++) {
    text_layer_destroy(s_digits[i]);
  }
  layer_destroy(s_date_layer);
  layer_destroy(s_bt_layer);
  gbitmap_destroy(s_bt_bitmap);

  window_destroy(s_window);
  s_window = NULL;
}

/*********************************** Reload Configuration Settings ************************************/

static void reload_config() {
  Layer *window_layer = window_get_root_layer(s_window);

  // Services
  tick_timer_service_unsubscribe();
  tick_timer_service_subscribe(data_get_boolean_setting(DataKeyAnimations) ? SECOND_UNIT : MINUTE_UNIT, tick_handler);
  if(data_get_boolean_setting(DataKeyBTIndicator)) {
    connection_service_subscribe((ConnectionHandlers) {
      .pebble_app_connection_handler = bt_handler
    });
  } else {
    connection_service_unsubscribe();
  }  

  // BT layer
  if(data_get_boolean_setting(DataKeyBTIndicator)) {
    layer_set_hidden(s_bt_layer, connection_service_peek_pebble_app_connection());
    // layer_set_hidden(s_bt_layer, false);
  } else {
    // Don't want this
    layer_set_hidden(s_bt_layer, true);
  }  

  // Colors
  window_set_background_color(s_window, data_get_background_color());
  for(int i = 0; i < NUM_CHARS; i++) {
    text_layer_set_text_color(s_digits[i], data_get_foreground_color());
  }
  // text_layer_set_text_color(s_date_layer, data_get_foreground_color());

  // Show date
  if(data_get_boolean_setting(DataKeyDate)) {
    layer_add_child(window_layer, s_date_layer);
  } else {
    layer_remove_from_parent(s_date_layer);
  }

  layer_mark_dirty(s_bt_layer);  
  layer_mark_dirty(s_date_layer);

}

static void reload_config_handler(void *context) {
  reload_config();
}

/************************************ API *************************************/

// Initialize the main window, gets called from main.c on app start up once
void main_window_push() {
  if(!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }
  window_stack_push(s_window, true);

  // Make sure the face is not blank
  time_t temp = time(NULL);
  struct tm *time_now = localtime(&temp);
  update_digit_values(time_now);
  show_digit_values();

  // Stop 'all change' on first minute
  for(int i = 0; i < NUM_CHARS - 1; i++) {
    s_digit_states_prev[i] = s_digit_states_now[i];
  }

  const bool delay = false;
  main_window_reload_config(delay);
}

void main_window_reload_config(bool delay) {
  if(delay) {
    // Avoid weird things happening mid beaming up
    const int wait_duration_ms = 2000;
    app_timer_register(wait_duration_ms, reload_config_handler, NULL);
  } else {
    reload_config();
  }
}
