#pragma once

#include <lvgl.h>
#include "theme.h"

namespace ui_widgets {

inline lv_obj_t *label(lv_obj_t *parent, const char *txt, uint8_t sz = 0, lv_color_t color = ui_theme::WHITE,
                       bool bold = false) {
  lv_obj_t *l = lv_label_create(parent);
  lv_label_set_text(l, txt);
  ui_theme::style_text(l, sz, color, bold);
  return l;
}

inline lv_obj_t *card(lv_obj_t *parent, lv_coord_t w = LV_SIZE_CONTENT, lv_coord_t h = LV_SIZE_CONTENT) {
  lv_obj_t *obj = lv_obj_create(parent);
  ui_theme::style_panel(obj);
  lv_obj_set_size(obj, w, h);
  lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
  return obj;
}

inline lv_obj_t *row(lv_obj_t *parent, lv_coord_t gap = 3) {
  lv_obj_t *r = lv_obj_create(parent);
  lv_obj_remove_style_all(r);
  lv_obj_set_size(r, LV_PCT(100), LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(r, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(r, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_column(r, gap, 0);
  return r;
}

inline lv_obj_t *bar(lv_obj_t *parent, uint8_t value = 0, lv_color_t color = ui_theme::GREEN) {
  lv_obj_t *b = lv_bar_create(parent);
  lv_obj_set_size(b, LV_PCT(100), 6);
  lv_bar_set_range(b, 0, 100);
  lv_bar_set_value(b, value, LV_ANIM_OFF);
  lv_obj_set_style_bg_color(b, ui_theme::BORDER, LV_PART_MAIN);
  lv_obj_set_style_bg_color(b, color, LV_PART_INDICATOR);
  lv_obj_set_style_radius(b, 1, LV_PART_MAIN | LV_PART_INDICATOR);
  return b;
}

inline lv_obj_t *dot(lv_obj_t *parent, lv_color_t color, lv_coord_t s = 6) {
  lv_obj_t *d = lv_obj_create(parent);
  lv_obj_remove_style_all(d);
  lv_obj_set_size(d, s, s);
  lv_obj_set_style_radius(d, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_opa(d, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(d, color, 0);
  return d;
}

} // namespace ui_widgets
