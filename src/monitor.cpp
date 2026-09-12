#include "monitor.h"
#include "state.h"
#include <string.h>

#define HIST_LEN 40

static uint8_t mem_hist[HIST_LEN] = {0};
static uint8_t cpu_hist[HIST_LEN] = {0};
static uint8_t hd_hist[HIST_LEN]  = {0};
static int hist_index = 0;
static int hist_count = 0;

static float last_mem_pct = 0.0f;
static float last_cpu_pct = 0.0f;
static float last_hd_pct  = 0.0f;

void monitor_init() {
    memset(mem_hist, 0, sizeof(mem_hist));
    memset(cpu_hist, 0, sizeof(cpu_hist));
    memset(hd_hist, 0, sizeof(hd_hist));
    hist_index = 0;
    hist_count = 0;
    last_mem_pct = 0.0f;
    last_cpu_pct = 0.0f;
    last_hd_pct  = 0.0f;
}

static uint8_t clamp_pct(float v) {
    if (v < 0.0f) v = 0.0f;
    if (v > 100.0f) v = 100.0f;
    return (uint8_t)v;
}

void monitor_update() {
    // valores atualizados pelo SerialReader (single-writer: serial_reader.cpp)
    last_cpu_pct = stats_cpu_pct;
    last_mem_pct = stats_mem_pct;
    last_hd_pct  = stats_disk_pct;

    mem_hist[hist_index] = clamp_pct(last_mem_pct);
    cpu_hist[hist_index] = clamp_pct(last_cpu_pct);
    hd_hist[hist_index]  = clamp_pct(last_hd_pct);
    hist_index = (hist_index + 1) % HIST_LEN;
    if (hist_count < HIST_LEN) hist_count++;
}

static void draw_graph(Adafruit_SSD1306 &disp, int x, int y, int w, int h,
                        uint8_t *hist, const char *label, float current_pct) {
    disp.drawRect(x, y, w, h, SSD1306_WHITE);

    char buf[24];
    snprintf(buf, sizeof(buf), "%s %d%%", label, (int)current_pct);
    disp.setTextSize(1);
    disp.setCursor(x, y - 8);
    disp.print(buf);

    int usable_w = w - 2;
    int usable_h = h - 2;
    int n = hist_count;
    if (n < 2) return;

    int start = (hist_index - n + HIST_LEN) % HIST_LEN;
    int prev_x = -1, prev_y = -1;
    for (int i = 0; i < n; i++) {
        int idx = (start + i) % HIST_LEN;
        int px = x + 1 + (int)((float)i / (HIST_LEN - 1) * usable_w);
        int val = hist[idx];
        int py = y + 1 + usable_h - (int)((val / 100.0f) * usable_h);
        if (prev_x >= 0) disp.drawLine(prev_x, prev_y, px, py, SSD1306_WHITE);
        prev_x = px;
        prev_y = py;
    }
}

void monitor_draw(Adafruit_SSD1306 &disp) {
    disp.clearDisplay();
    disp.setTextColor(SSD1306_WHITE);

    draw_graph(disp, 2, 9,  124, 12, mem_hist, "MEM", last_mem_pct);
    draw_graph(disp, 2, 31, 124, 12, cpu_hist, "CPU", last_cpu_pct);
    draw_graph(disp, 2, 53, 124, 10, hd_hist,  "HD",  last_hd_pct);

    disp.display();
}