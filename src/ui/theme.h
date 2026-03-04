#pragma once

#include <lvgl.h>

namespace ui_theme {

static constexpr lv_color_t BG = lv_color_hex(0x000000);
static constexpr lv_color_t SURFACE = lv_color_hex(0x0A0A0A);
static constexpr lv_color_t PANEL = lv_color_hex(0x0D0F12);
static constexpr lv_color_t BORDER = lv_color_hex(0x1A1E24);
static constexpr lv_color_t BORDER_HI = lv_color_hex(0x252C35);
static constexpr lv_color_t WHITE = lv_color_hex(0xFFFFFF);
static constexpr lv_color_t GRAY = lv_color_hex(0xAAAAAA);
static constexpr lv_color_t GRAY_DIM = lv_color_hex(0x555555);
static constexpr lv_color_t GRAY_DARK = lv_color_hex(0x1A1A1A);
static constexpr lv_color_t ARMED = lv_color_hex(0xFF2200);
static constexpr lv_color_t ARMED_DIM = lv_color_hex(0x2A0500);
static constexpr lv_color_t DISARMED = lv_color_hex(0x0023FF);
static constexpr lv_color_t DISARMED_DIM = lv_color_hex(0x000A30);
static constexpr lv_color_t SIG_LOST = lv_color_hex(0xFFCC00);
static constexpr lv_color_t SIG_LOST_DIM = lv_color_hex(0x2A2000);
static constexpr lv_color_t GREEN = lv_color_hex(0x00FF66);
static constexpr lv_color_t GREEN_DIM = lv_color_hex(0x003318);
static constexpr lv_color_t CYAN = lv_color_hex(0x00CCFF);
static constexpr lv_color_t RED = lv_color_hex(0xFF3333);

inline void style_panel(lv_obj_t *obj) {
  lv_obj_set_style_bg_color(obj, PANEL, 0);
  lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
  lv_obj_set_style_border_color(obj, BORDER, 0);
  lv_obj_set_style_border_width(obj, 1, 0);
  lv_obj_set_style_radius(obj, 2, 0);
  lv_obj_set_style_pad_all(obj, 3, 0);
}

inline void style_card_glow(lv_obj_t *obj, lv_color_t border, lv_color_t glow) {
  style_panel(obj);
  lv_obj_set_style_border_color(obj, border, 0);
  lv_obj_set_style_shadow_color(obj, glow, 0);
  lv_obj_set_style_shadow_opa(obj, LV_OPA_40, 0);
  lv_obj_set_style_shadow_width(obj, 10, 0);
}

inline const lv_font_t *font_sz(uint8_t sz_index) {
  switch (sz_index) {
  case 0: return &lv_font_montserrat_8;
  case 1: return &lv_font_montserrat_10;
  case 2: return &lv_font_montserrat_12;
  case 3: return &lv_font_montserrat_14;
  case 4: return &lv_font_montserrat_18;
  case 5: return &lv_font_montserrat_22;
  default: return &lv_font_montserrat_28;
  }
}

inline void style_text(lv_obj_t *obj, uint8_t sz, lv_color_t color, bool bold = false) {
  lv_obj_set_style_text_font(obj, font_sz(sz), 0);
  lv_obj_set_style_text_color(obj, color, 0);
  if (bold) lv_obj_set_style_text_letter_space(obj, 1, 0);
}

} // namespace ui_theme
