#include "ui.h"

#include <stdio.h>
#include <cstring>

#include "theme.h"
#include "widgets.h"

namespace {

UiData g_data{};
ScreenId g_screen = ScreenId::BOOT;
bool g_frozen = false;
bool g_estop = false;
bool g_log_modal = false;
bool g_comp_detail = false;
uint32_t g_boot_progress = 0;
uint32_t g_last_step_ms = 0;
uint8_t g_auto_idx = 0;

lv_obj_t *g_root = nullptr;
lv_obj_t *g_status = nullptr;
lv_obj_t *g_freeze = nullptr;
lv_obj_t *g_content = nullptr;
lv_obj_t *g_log_overlay = nullptr;
lv_obj_t *g_estop_overlay = nullptr;

lv_obj_t *g_boot_cont = nullptr;
lv_obj_t *g_primary_cont = nullptr;
lv_obj_t *g_diag_cont = nullptr;
lv_obj_t *g_logs_cont = nullptr;
lv_obj_t *g_auto_cont = nullptr;
lv_obj_t *g_comp_cont = nullptr;

lv_obj_t *lbl_status_l = nullptr;
lv_obj_t *lbl_status_r = nullptr;
lv_obj_t *lbl_boot_steps = nullptr;
lv_obj_t *bar_boot = nullptr;

lv_obj_t *lbl_primary_state = nullptr;
lv_obj_t *bar_batt = nullptr;
lv_obj_t *lbl_batt = nullptr;
lv_obj_t *lbl_pkt = nullptr;
lv_obj_t *lbl_age = nullptr;
lv_obj_t *lbl_drive_l = nullptr;
lv_obj_t *lbl_drive_r = nullptr;

lv_obj_t *lbl_diag_info = nullptr;
lv_obj_t *lbl_diag_rows = nullptr;

lv_obj_t *lbl_logs = nullptr;
lv_obj_t *lbl_auto = nullptr;
lv_obj_t *lbl_comp = nullptr;

const char *state_name(RobotState s) {
  switch (s) {
  case RobotState::SIGNAL_LOST: return "SIG LOST";
  case RobotState::DISARMED: return "DISARMED";
  default: return "ARMED";
  }
}

lv_color_t state_color(RobotState s) {
  switch (s) {
  case RobotState::SIGNAL_LOST: return ui_theme::SIG_LOST;
  case RobotState::DISARMED: return ui_theme::DISARMED;
  default: return ui_theme::ARMED;
  }
}

void show_screen(ScreenId s) {
  g_screen = s;
  lv_obj_add_flag(g_boot_cont, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(g_primary_cont, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(g_diag_cont, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(g_logs_cont, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(g_auto_cont, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(g_comp_cont, LV_OBJ_FLAG_HIDDEN);

  lv_obj_clear_flag(s == ScreenId::BOOT ? g_boot_cont :
                    s == ScreenId::PRIMARY ? g_primary_cont :
                    s == ScreenId::DIAG ? g_diag_cont :
                    s == ScreenId::LOGS ? g_logs_cont :
                    s == ScreenId::AUTOTEST ? g_auto_cont : g_comp_cont,
                    LV_OBJ_FLAG_HIDDEN);
}

void make_statusbar() {
  g_status = lv_obj_create(g_root);
  lv_obj_set_pos(g_status, 0, 0);
  lv_obj_set_size(g_status, 320, 11);
  lv_obj_clear_flag(g_status, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(g_status, ui_theme::PANEL, 0);
  lv_obj_set_style_border_color(g_status, ui_theme::BORDER, 0);
  lv_obj_set_style_border_side(g_status, LV_BORDER_SIDE_BOTTOM, 0);
  lv_obj_set_style_pad_all(g_status, 1, 0);
  lv_obj_set_flex_flow(g_status, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(g_status, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  lbl_status_l = ui_widgets::label(g_status, "BRL · STATUS", 0, ui_theme::GRAY);
  lbl_status_r = ui_widgets::label(g_status, "DISARMED | 24.7V | ELRS", 0, ui_theme::GRAY);
}

void make_overlays() {
  g_freeze = lv_obj_create(g_root);
  lv_obj_set_pos(g_freeze, 0, 11);
  lv_obj_set_size(g_freeze, 320, 10);
  lv_obj_set_style_bg_color(g_freeze, ui_theme::SIG_LOST_DIM, 0);
  lv_obj_set_style_border_color(g_freeze, ui_theme::SIG_LOST, 0);
  lv_obj_set_style_border_side(g_freeze, LV_BORDER_SIDE_BOTTOM, 0);
  lv_obj_clear_flag(g_freeze, LV_OBJ_FLAG_SCROLLABLE);
  ui_widgets::label(g_freeze, "FROZEN", 0, ui_theme::SIG_LOST, true);
  lv_obj_add_flag(g_freeze, LV_OBJ_FLAG_HIDDEN);

  g_log_overlay = lv_obj_create(g_root);
  lv_obj_set_size(g_log_overlay, 320, 240);
  lv_obj_set_style_bg_color(g_log_overlay, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(g_log_overlay, LV_OPA_80, 0);
  lv_obj_set_style_border_width(g_log_overlay, 0, 0);
  lv_obj_center(g_log_overlay);
  lv_obj_t *box = ui_widgets::card(g_log_overlay, 180, 70);
  lv_obj_center(box);
  lv_obj_set_style_border_color(box, ui_theme::GREEN, 0);
  lv_obj_set_flex_flow(box, LV_FLEX_FLOW_COLUMN);
  ui_widgets::label(box, "✓ FRAME LOGGED", 1, ui_theme::GREEN, true);
  ui_widgets::label(box, "RISLink/NexBus · 115200", 0, ui_theme::GRAY_DIM);
  lv_obj_add_flag(g_log_overlay, LV_OBJ_FLAG_HIDDEN);

  g_estop_overlay = lv_obj_create(g_root);
  lv_obj_set_size(g_estop_overlay, 320, 240);
  lv_obj_set_style_bg_color(g_estop_overlay, lv_color_hex(0x0D0000), 0);
  lv_obj_set_style_border_width(g_estop_overlay, 0, 0);
  lv_obj_t *warn = ui_widgets::label(g_estop_overlay, "E-STOP\nROBOT DISABLED", 4, ui_theme::ARMED, true);
  lv_obj_set_style_text_align(warn, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_center(warn);
  lv_obj_add_flag(g_estop_overlay, LV_OBJ_FLAG_HIDDEN);
}

void make_boot() {
  g_boot_cont = lv_obj_create(g_content);
  lv_obj_remove_style_all(g_boot_cont);
  lv_obj_set_size(g_boot_cont, 320, 229);
  lv_obj_set_style_bg_color(g_boot_cont, ui_theme::BG, 0);
  lv_obj_set_style_bg_opa(g_boot_cont, LV_OPA_COVER, 0);

  lv_obj_t *icon = lv_obj_create(g_boot_cont);
  lv_obj_set_size(icon, 52, 36);
  lv_obj_set_style_bg_color(icon, ui_theme::DISARMED, 0);
  lv_obj_set_style_radius(icon, 2, 0);
  lv_obj_set_style_border_width(icon, 0, 0);
  lv_obj_align(icon, LV_ALIGN_TOP_MID, 0, 32);
  ui_widgets::label(g_boot_cont, "RDT-H1", 3, ui_theme::DISARMED, true);
  lv_obj_align_to(lv_obj_get_child(g_boot_cont, 1), icon, LV_ALIGN_OUT_BOTTOM_MID, 0, 6);
  lbl_boot_steps = ui_widgets::label(g_boot_cont, "INIT UART 115200", 0, ui_theme::GRAY);
  lv_obj_align(lbl_boot_steps, LV_ALIGN_TOP_LEFT, 40, 95);
  bar_boot = ui_widgets::bar(g_boot_cont, 0, ui_theme::DISARMED);
  lv_obj_set_width(bar_boot, 240);
  lv_obj_align(bar_boot, LV_ALIGN_TOP_LEFT, 40, 145);
  ui_widgets::label(g_boot_cont, "RISLink/NexBus · UART 115200", 0, ui_theme::GRAY_DIM);
  lv_obj_align(lv_obj_get_child(g_boot_cont, 4), LV_ALIGN_TOP_LEFT, 40, 158);
}

void make_primary() {
  g_primary_cont = lv_obj_create(g_content);
  lv_obj_remove_style_all(g_primary_cont);
  lv_obj_set_size(g_primary_cont, 320, 229);
  lv_obj_set_style_pad_all(g_primary_cont, 4, 0);
  lv_obj_set_flex_flow(g_primary_cont, LV_FLEX_FLOW_COLUMN);

  lv_obj_t *hero = ui_widgets::card(g_primary_cont, LV_PCT(100), 42);
  ui_theme::style_card_glow(hero, ui_theme::DISARMED, ui_theme::DISARMED);
  lbl_primary_state = ui_widgets::label(hero, "DISARMED", 4, ui_theme::DISARMED, true);
  ui_widgets::label(hero, "ROBOT STATE", 0, ui_theme::GRAY_DIM);

  lv_obj_t *row1 = ui_widgets::row(g_primary_cont, 3);
  lv_obj_t *batt = ui_widgets::card(row1, 154, 30);
  ui_widgets::label(batt, "BATTERY", 0, ui_theme::GRAY_DIM);
  lbl_batt = ui_widgets::label(batt, "24.7V", 2, ui_theme::GREEN, true);
  bar_batt = ui_widgets::bar(batt, 80, ui_theme::GREEN);
  lv_obj_set_width(bar_batt, 140);

  lv_obj_t *link = ui_widgets::card(row1, 154, 30);
  ui_widgets::label(link, "ELRS LINK", 0, ui_theme::GRAY_DIM);
  lbl_pkt = ui_widgets::label(link, "150fps", 2, ui_theme::GREEN, true);
  lbl_age = ui_widgets::label(link, "AGE 8ms", 0, ui_theme::GRAY);

  lv_obj_t *rc = ui_widgets::card(g_primary_cont, LV_PCT(100), 70);
  ui_widgets::label(rc, "RC CHANNELS  ARM:LOW", 0, ui_theme::GRAY_DIM);
  ui_widgets::label(rc, "L STICK [ ]  CH1 CH2 CH3 CH4  R STICK [ ]", 0, ui_theme::GRAY);

  lv_obj_t *drv = ui_widgets::card(g_primary_cont, LV_PCT(100), 42);
  ui_widgets::label(drv, "DRIVE OUTPUTS", 0, ui_theme::GRAY_DIM);
  lbl_drive_l = ui_widgets::label(drv, "LEFT 1500us", 0, ui_theme::GRAY);
  lbl_drive_r = ui_widgets::label(drv, "RIGHT 1500us", 0, ui_theme::GRAY);
  ui_widgets::label(g_primary_cont, "▼ DIAG      LOG · FREEZE · E-STOP", 0, ui_theme::GRAY_DIM);
}

void make_diag() {
  g_diag_cont = lv_obj_create(g_content);
  lv_obj_remove_style_all(g_diag_cont);
  lv_obj_set_size(g_diag_cont, 320, 229);
  lv_obj_set_style_pad_all(g_diag_cont, 4, 0);
  lv_obj_set_flex_flow(g_diag_cont, LV_FLEX_FLOW_COLUMN);
  ui_widgets::label(g_diag_cont, "DIAGNOSTICS", 1, ui_theme::DISARMED, true);
  lbl_diag_info = ui_widgets::label(g_diag_cont, "NexCore CB · v0.4.2 · REV-A", 0, ui_theme::GRAY);
  lv_obj_t *box = ui_widgets::card(g_diag_cont, LV_PCT(100), 170);
  lbl_diag_rows = ui_widgets::label(box, "", 0, ui_theme::GRAY);
  lv_obj_set_style_text_line_space(lbl_diag_rows, 2, 0);
  ui_widgets::label(g_diag_cont, "▲ STATUS  ▼ LOGS      BACK", 0, ui_theme::GRAY_DIM);
}

void make_logs() {
  g_logs_cont = lv_obj_create(g_content);
  lv_obj_remove_style_all(g_logs_cont);
  lv_obj_set_size(g_logs_cont, 320, 229);
  lv_obj_set_style_pad_all(g_logs_cont, 4, 0);
  lv_obj_set_flex_flow(g_logs_cont, LV_FLEX_FLOW_COLUMN);
  ui_widgets::label(g_logs_cont, "LOG REVIEW", 1, ui_theme::DISARMED, true);
  ui_widgets::label(g_logs_cont, "ALL  STATE  FAULT  INFO", 0, ui_theme::GRAY_DIM);
  lv_obj_t *box = ui_widgets::card(g_logs_cont, LV_PCT(100), 178);
  lbl_logs = ui_widgets::label(box, "", 0, ui_theme::GRAY);
  lv_obj_set_style_text_line_space(lbl_logs, 1, 0);
  ui_widgets::label(g_logs_cont, "▲▼ SCROLL  SEL DETAIL  ▼ AUTO TEST", 0, ui_theme::GRAY_DIM);
}

void make_auto() {
  g_auto_cont = lv_obj_create(g_content);
  lv_obj_remove_style_all(g_auto_cont);
  lv_obj_set_size(g_auto_cont, 320, 229);
  lv_obj_set_style_pad_all(g_auto_cont, 4, 0);
  lv_obj_set_flex_flow(g_auto_cont, LV_FLEX_FLOW_COLUMN);
  ui_widgets::label(g_auto_cont, "AUTO TEST", 1, ui_theme::DISARMED, true);
  lv_obj_t *box = ui_widgets::card(g_auto_cont, LV_PCT(100), 188);
  lbl_auto = ui_widgets::label(box, "", 0, ui_theme::GRAY);
  lv_obj_set_style_text_line_space(lbl_auto, 2, 0);
  ui_widgets::label(g_auto_cont, "SEL RESULT  ▼ COMP TEST     ▶ RUN ALL", 0, ui_theme::GRAY_DIM);
}

void make_comp() {
  g_comp_cont = lv_obj_create(g_content);
  lv_obj_remove_style_all(g_comp_cont);
  lv_obj_set_size(g_comp_cont, 320, 229);
  lv_obj_set_style_pad_all(g_comp_cont, 4, 0);
  lv_obj_set_flex_flow(g_comp_cont, LV_FLEX_FLOW_COLUMN);
  ui_widgets::label(g_comp_cont, "COMP TEST", 1, ui_theme::DISARMED, true);
  lv_obj_t *box = ui_widgets::card(g_comp_cont, LV_PCT(100), 188);
  lbl_comp = ui_widgets::label(box, "", 0, ui_theme::GRAY);
  lv_obj_set_style_text_line_space(lbl_comp, 2, 0);
  ui_widgets::label(g_comp_cont, "▲ AUTO TEST  SEL DRILL    ▶ RUN", 0, ui_theme::GRAY_DIM);
}

void update_status() {
  const char *screen_label = g_screen == ScreenId::PRIMARY ? "STATUS" :
                             g_screen == ScreenId::DIAG ? "DIAG" :
                             g_screen == ScreenId::LOGS ? "LOGS" :
                             g_screen == ScreenId::AUTOTEST ? "AUTO TEST" :
                             g_screen == ScreenId::COMPTEST ? "COMP TEST" : "BOOT";
  char left[48];
  snprintf(left, sizeof(left), "BRL · %s%s", screen_label, g_frozen ? " · *" : "");
  lv_label_set_text(lbl_status_l, left);

  char right[64];
  if (g_estop) {
    lv_label_set_text(lbl_status_r, "■ E-STOP");
    lv_obj_set_style_bg_color(g_status, ui_theme::ARMED_DIM, 0);
    lv_obj_set_style_border_color(g_status, ui_theme::ARMED, 0);
    lv_obj_set_style_text_color(lbl_status_r, ui_theme::WHITE, 0);
  } else {
    snprintf(right, sizeof(right), "%s | %.1fV | %s", state_name(g_data.robot_state), g_data.voltage_mv / 1000.0f,
             g_data.elrs_connected ? "ELRS" : "NO RX");
    lv_label_set_text(lbl_status_r, right);
    lv_obj_set_style_bg_color(g_status, ui_theme::PANEL, 0);
    lv_obj_set_style_border_color(g_status, ui_theme::BORDER, 0);
    lv_obj_set_style_text_color(lbl_status_r, state_color(g_data.robot_state), 0);
  }
}

} // namespace

void ui_init(lv_obj_t *root) {
  g_root = root;
  lv_obj_set_size(g_root, 320, 240);
  lv_obj_set_style_bg_color(g_root, ui_theme::BG, 0);
  lv_obj_set_style_pad_all(g_root, 0, 0);
  lv_obj_clear_flag(g_root, LV_OBJ_FLAG_SCROLLABLE);

  make_statusbar();

  g_content = lv_obj_create(g_root);
  lv_obj_remove_style_all(g_content);
  lv_obj_set_pos(g_content, 0, 11);
  lv_obj_set_size(g_content, 320, 229);

  make_boot();
  make_primary();
  make_diag();
  make_logs();
  make_auto();
  make_comp();
  make_overlays();

  show_screen(ScreenId::BOOT);
  ui_update();
}

void ui_set_data(const UiData &data) { g_data = data; }

void ui_update() {
  update_status();
  if (!g_frozen || g_estop) lv_obj_add_flag(g_freeze, LV_OBJ_FLAG_HIDDEN); else lv_obj_clear_flag(g_freeze, LV_OBJ_FLAG_HIDDEN);
  if (!g_log_modal) lv_obj_add_flag(g_log_overlay, LV_OBJ_FLAG_HIDDEN); else lv_obj_clear_flag(g_log_overlay, LV_OBJ_FLAG_HIDDEN);
  if (!g_estop) lv_obj_add_flag(g_estop_overlay, LV_OBJ_FLAG_HIDDEN); else lv_obj_clear_flag(g_estop_overlay, LV_OBJ_FLAG_HIDDEN);

  const char *steps = g_boot_progress < 20 ? "INIT UART 115200" :
                      g_boot_progress < 40 ? "INIT UART\nNEXBUS HANDSHAKE" :
                      g_boot_progress < 60 ? "INIT UART\nNEXBUS HANDSHAKE\nNEXCORE CB FOUND" :
                      g_boot_progress < 80 ? "...\nSYNC v0.4.2" : "...\nREADY";
  lv_label_set_text(lbl_boot_steps, steps);
  lv_bar_set_value(bar_boot, g_boot_progress, LV_ANIM_OFF);

  lv_label_set_text(lbl_primary_state, state_name(g_data.robot_state));
  lv_obj_set_style_text_color(lbl_primary_state, state_color(g_data.robot_state), 0);
  int batt = (int)((g_data.voltage_mv - 20000) * 100 / 5200);
  if (batt < 0) batt = 0; if (batt > 100) batt = 100;
  char line[64];
  snprintf(line, sizeof(line), "%.1fV", g_data.voltage_mv / 1000.0f);
  lv_label_set_text(lbl_batt, line);
  lv_bar_set_value(bar_batt, batt, LV_ANIM_OFF);
  snprintf(line, sizeof(line), "%dfps", g_data.packet_rate_fps);
  lv_label_set_text(lbl_pkt, line);
  snprintf(line, sizeof(line), "AGE %ums", g_data.signal_age_ms);
  lv_label_set_text(lbl_age, line);
  snprintf(line, sizeof(line), "LEFT %uus", g_data.pwm_left_us);
  lv_label_set_text(lbl_drive_l, line);
  snprintf(line, sizeof(line), "RIGHT %uus", g_data.pwm_right_us);
  lv_label_set_text(lbl_drive_r, line);

  snprintf(line, sizeof(line), "%s · %s · %s", g_data.model_name, g_data.firmware_version, g_data.hardware_revision);
  lv_label_set_text(lbl_diag_info, line);
  char diag[512];
  snprintf(diag, sizeof(diag),
           "ELRS STATUS: %s\nPACKET RATE: %u fps\nSIGNAL AGE: %u ms\nARM STATE: %s\n"
           "MOTOR GATE: %s\nBATT MV: %lu\nADC RAW: %u\nUPTIME: %lus\nDEBUG: %s",
           g_data.elrs_connected ? "CONNECTED" : "LOST", g_data.packet_rate_fps, g_data.signal_age_ms,
           state_name(g_data.robot_state), g_data.motor_gate_open ? "OPEN" : "HARD-GATED", g_data.voltage_mv,
           g_data.adc_raw, (unsigned long)(g_data.uptime_ms / 1000), g_data.debug_mode ? "ON" : "OFF");
  lv_label_set_text(lbl_diag_rows, diag);

  lv_label_set_text(lbl_logs,
                    "12:14:02.211 STATE DISARMED->ARMED\n"
                    "12:14:22.441 INFO Motor gate open\n"
                    "12:14:31.999 FLT  LOW BATTERY\n"
                    "12:14:39.104 STATE ARMED->DISARMED\n"
                    "12:14:52.552 STATE DISARMED->SIG LOST\n"
                    "12:14:58.120 FLT  ELRS link lost\n"
                    "12:15:03.774 STATE SIG LOST->DISARMED\n"
                    "12:15:25.812 INFO Rearm guard active\n"
                    "12:15:42.702 FLT  Packet rate drop\n"
                    "12:15:44.015 INFO Recovered 148fps\n"
                    "12:15:50.100 STATE DISARMED->ARMED");

  const char *auto_tests[] = {
    "ELRS Link Quality", "Motor Gate Cycle", "Battery Cal Check",
    "Arm/Disarm Sequence", "Watchdog Timeout", "Channel Range"
  };
  char auto_buf[512] = {0};
  for (uint8_t i = 0; i < 6; i++) {
    const char *st = i < g_auto_idx ? "PASS" : i == g_auto_idx ? "RUN·" : "--";
    char row[80];
    snprintf(row, sizeof(row), "%s  %s\n", auto_tests[i], st);
    strncat(auto_buf, row, sizeof(auto_buf) - strlen(auto_buf) - 1);
  }
  lv_label_set_text(lbl_auto, auto_buf);

  if (!g_comp_detail) {
    lv_label_set_text(lbl_comp,
                      "SUMMARY\nELRS RX      PASS\nMOTOR GATE   PASS\nBATT ADC     FAIL\n"
                      "ARM SW       PASS\nUART         PASS\nWATCHDOG     PASS\nCH DECODE    PASS\n"
                      "FAILURES: BATT ADC");
  } else {
    lv_label_set_text(lbl_comp,
                      "DETAIL: BATT ADC FAIL\n✓ Voltage >18V\n✓ ADC in range\n✗ Cal error <100\n✓ Low batt flag\n"
                      "FAIL NAV: [BATT_ADC]");
  }
}

void ui_next_screen() {
  if (g_screen == ScreenId::BOOT) show_screen(ScreenId::PRIMARY);
  else if (g_screen == ScreenId::PRIMARY) show_screen(ScreenId::DIAG);
  else if (g_screen == ScreenId::DIAG) show_screen(ScreenId::LOGS);
  else if (g_screen == ScreenId::LOGS) show_screen(ScreenId::AUTOTEST);
  else if (g_screen == ScreenId::AUTOTEST) show_screen(ScreenId::COMPTEST);
}

void ui_prev_screen() {
  if (g_screen == ScreenId::COMPTEST) show_screen(ScreenId::AUTOTEST);
  else if (g_screen == ScreenId::AUTOTEST) show_screen(ScreenId::LOGS);
  else if (g_screen == ScreenId::LOGS) show_screen(ScreenId::DIAG);
  else if (g_screen == ScreenId::DIAG) show_screen(ScreenId::PRIMARY);
}

void ui_back() { ui_prev_screen(); }
void ui_toggle_freeze() { if (!g_estop) g_frozen = !g_frozen; }
void ui_trigger_estop() { g_estop = true; g_frozen = false; }
void ui_resume_from_estop() { g_estop = false; show_screen(ScreenId::PRIMARY); }
void ui_log_frame() { if (!g_estop) g_log_modal = true; }
void ui_run_auto_tests() { g_auto_idx = 0; }
void ui_run_comp_tests() { g_comp_detail = false; }

void ui_demo_tick() {
  const uint32_t now = lv_tick_get();
  if (g_screen == ScreenId::BOOT) {
    if (now - g_last_step_ms > 50 && g_boot_progress < 100) {
      g_boot_progress++;
      g_last_step_ms = now;
    }
    if (g_boot_progress >= 100) show_screen(ScreenId::PRIMARY);
  }

  if (now % 7000 < 30 && g_screen != ScreenId::BOOT && !g_estop) ui_next_screen();
  if (now % 15000 < 30) g_frozen = !g_frozen;
  if (now % 20000 < 30) ui_log_frame();
  if (now % 32000 < 30) ui_trigger_estop();
  if (g_estop && now % 32000 > 3000 && now % 32000 < 3030) ui_resume_from_estop();
  if (g_log_modal && now % 20000 > 1800) g_log_modal = false;

  if (g_screen == ScreenId::AUTOTEST && (now % 1000 < 30)) g_auto_idx = (g_auto_idx + 1) % 7;
  if (g_screen == ScreenId::COMPTEST && (now % 6000 < 30)) g_comp_detail = !g_comp_detail;

  ui_update();
}
