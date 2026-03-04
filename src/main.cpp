#include <Arduino.h>
#include <TFT_eSPI.h>
#include <lvgl.h>

#include "ui/ui.h"

static TFT_eSPI tft = TFT_eSPI();
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[320 * 20];

static UiData sim_data = {
    "NexCore CB", "v0.4.2", "REV-A", "2025-02-20 14:32", RobotState::DISARMED, true,
    1500, 1500, 1000, 1500, 1000, 8, 150,
    1500, 1500, false,
    24700, false, 3021, 10000, 3300,
    false, false, false, false, 0, 0, false};

static void lv_flush_cb(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  const uint32_t w = area->x2 - area->x1 + 1;
  const uint32_t h = area->y2 - area->y1 + 1;
  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors(reinterpret_cast<uint16_t *>(color_p), w * h, true);
  tft.endWrite();
  lv_disp_flush_ready(disp);
}

static void lv_tick_task(void *arg) {
  (void)arg;
  lv_tick_inc(5);
}

void setup() {
  Serial.begin(115200);
  tft.begin();
  tft.setRotation(1);

  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf1, nullptr, 320 * 20);

  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = 320;
  disp_drv.ver_res = 240;
  disp_drv.flush_cb = lv_flush_cb;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  const esp_timer_create_args_t timer_args = {.callback = &lv_tick_task, .name = "lv_tick"};
  esp_timer_handle_t timer;
  esp_timer_create(&timer_args, &timer);
  esp_timer_start_periodic(timer, 5000);

  lv_obj_t *screen = lv_scr_act();
  ui_init(screen);
}

void loop() {
  static uint32_t last = 0;
  const uint32_t now = millis();
  if (now - last > 200) {
    last = now;
    sim_data.uptime_ms += 200;
    sim_data.voltage_mv = 24600 + ((now / 200) % 10) * 8;
    sim_data.packet_rate_fps = 148 + ((now / 1000) % 4);
    sim_data.signal_age_ms = 6 + ((now / 500) % 6);
    sim_data.low_battery = sim_data.voltage_mv < 21000;
    sim_data.robot_state = static_cast<RobotState>((now / 6000) % 3);
    sim_data.ch5_us = sim_data.robot_state == RobotState::ARMED ? 2000 : 1000;
    sim_data.motor_gate_open = sim_data.robot_state == RobotState::ARMED;
    sim_data.pwm_left_us = sim_data.motor_gate_open ? 1600 : 1500;
    sim_data.pwm_right_us = sim_data.motor_gate_open ? 1400 : 1500;
    ui_set_data(sim_data);
  }

  ui_demo_tick();
  lv_timer_handler();
  delay(5);
}
