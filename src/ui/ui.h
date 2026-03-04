#pragma once

#include <lvgl.h>
#include <stdint.h>

enum class RobotState : uint8_t { SIGNAL_LOST, DISARMED, ARMED };
enum class ScreenId : uint8_t { BOOT, PRIMARY, DIAG, LOGS, AUTOTEST, COMPTEST };

struct UiData {
  const char *model_name;
  const char *firmware_version;
  const char *hardware_revision;
  const char *build_timestamp;
  RobotState robot_state;
  bool elrs_connected;

  uint16_t ch1_us;
  uint16_t ch2_us;
  uint16_t ch3_us;
  uint16_t ch4_us;
  uint16_t ch5_us;
  uint16_t signal_age_ms;
  uint16_t packet_rate_fps;

  uint16_t pwm_left_us;
  uint16_t pwm_right_us;
  bool motor_gate_open;

  uint32_t voltage_mv;
  bool low_battery;
  uint16_t adc_raw;
  uint16_t divider_r1;
  uint16_t divider_r2;

  bool weapon_esc_pin;
  bool servo_esc_pin;
  bool rislink_connected;
  bool rearm_guard;
  uint8_t aux5_position;
  uint32_t uptime_ms;
  bool debug_mode;
};

void ui_init(lv_obj_t *root);
void ui_set_data(const UiData &data);
void ui_update();

void ui_next_screen();
void ui_prev_screen();
void ui_back();
void ui_toggle_freeze();
void ui_trigger_estop();
void ui_resume_from_estop();
void ui_log_frame();
void ui_run_auto_tests();
void ui_run_comp_tests();

// Timer-driven demo sequence hook.
void ui_demo_tick();
