#include "mbed.h"
#include "TS_DISCO_F469NI.h"
#include "LCD_DISCO_F469NI.h"
#include "SD_DISCO_F469NI.h"
#include "neopixel.h"
#include "dotstar.h"
#include "framebuffer.h"
#include "SDFileSystem.h"
#include "background.h"
#include "gui.h"
#include "display_settings.h"
#include "motor_control.h"
#include "joystick.h"
#include <functional>
#include "pong.h"
#include "snowfall.h"
#include "earth.h"

void stop_calibration();
void start_calibration();
void stripey(render::framebuffer& buffer);

LCD_DISCO_F469NI lcd;
TS_DISCO_F469NI ts;
SDFileSystem sd("sd");
Timer t;
gui::interface ui(lcd, 6);

static constexpr uint32_t BACKGROUND_COLOUR = 0xFFED0000;
static constexpr uint32_t SCREEN_WIDTH = 800;
static constexpr uint32_t SCREEN_HEIGHT = 480;
// These are approximates just to give decent spacing
static constexpr uint32_t BORDER_HEIGHT = 60;
static constexpr uint32_t BORDER_WIDTH = 120;

render::framebuffer outer_buffer(OUTER_WIDTH, OUTER_HEIGHT);
render::framebuffer inner_buffer(INNER_WIDTH, INNER_HEIGHT);

app::pong game(inner_buffer);
app::snowfall<100> snow(outer_buffer);

size_t inner_offset = 24;
size_t outer_offset = 17;
size_t temp_rotation = 0;

std::function<void(void)> application_update;
std::function<void(void)> critical_update;

void critical_run() 
{
    // THIS MUST BE GUARANTEED TO BE INITIALISED
    critical_update();
}

void application_run()
{
    if(application_update)
        application_update();
}

bool init_sd_card() {
    return sd.disk_initialize() == 0;
}

void prepare_background()
{
    const uint32_t margin = 30;
    const uint32_t spacing = 5;
    for(uint32_t i=margin; i<SCREEN_WIDTH-margin; i+=(spacing+BORDER_WIDTH))
    {
        lcd.DrawBitmap(i, margin, resources_border_top_bmp);
        lcd.DrawBitmap(i, SCREEN_HEIGHT-BORDER_HEIGHT-margin, resources_border_bottom_bmp);
    }
}

void launch_pong()
{
    motors::set_state(motors::state::spin);
    outer_buffer.clear(ds::BLACK);
    snow.init();
    game.reset();
    application_update = [&]() {
        snow.update();
        game.update();
    };
}


void calibrate()
{
    using namespace app;
    if(stick_1.x_state() == x_motion::left) 
    {
        inner_offset--;
        if(inner_offset == -1) {
            inner_offset = INNER_WIDTH-1;
        } 
    }
    else if(stick_1.x_state() == x_motion::right)
    {
        inner_offset++;
        if(inner_offset == INNER_WIDTH) {
            inner_offset = 0;
        }
    }
    if(stick_2.x_state() == x_motion::right) 
    {
        outer_offset--;
        if(outer_offset == -1) {
            outer_offset = OUTER_WIDTH-1;
        }
    }
    else if(stick_2.x_state() == x_motion::left)
    {
        outer_offset++;
        if(outer_offset == OUTER_WIDTH)
            outer_offset = 0;
    }
}

void stop_calibration()
{
    motors::set_state(motors::state::stop);
    ui.get_button(0).text = "Calib";
    ui.get_button(0).action = start_calibration;
    ui.get_button(0).render(lcd);
    application_update = std::function<void(void)>();
}


void update_earth() 
{

    if(app::stick_1.x_state() == app::x_motion::left) 
    {
        temp_rotation--;
        if(temp_rotation == -1) {
            temp_rotation = INNER_WIDTH-1;
        } 
    }
    else if(app::stick_1.x_state() == app::x_motion::right)
    {
        temp_rotation++;
        if(temp_rotation == INNER_WIDTH) {
            temp_rotation = 0;
        }
    }
    snow.update();
}

void show_earth() { 
    int i =0;
    for(int c=0; c<INNER_WIDTH; c++) {
        for(int r=0; r<INNER_HEIGHT; r++) {
            inner_buffer.pixel_at(r, c) = EARTH[i];
            i++;
        }
    }
    temp_rotation = inner_offset;
    inner_buffer.swap();
    outer_buffer.clear(ds::BLACK);
    snow.init();
    application_update = update_earth;
    motors::set_state(motors::state::spin);
}

void start_calibration()
{
    // Setup calibration pattern
    outer_buffer.clear(ds::BLACK);
    inner_buffer.clear(ds::BLACK);
    outer_buffer.fill_rect(0, 0, 1, outer_buffer.n_row(), ds::BLUE);
    inner_buffer.fill_rect(0, 0, 1, inner_buffer.n_row(), ds::BLUE);
    // Setup stop calibration button

    application_update = calibrate;
    // Spin to win
    motors::set_state(motors::state::spin);
}

void init_neopixels()
{
    np::init_all();
    uint32_t data[] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    np::render_segment(data, 3);
}


void setup_main_menu(gui::interface& ui)
{
    // Assume 3 apps and hack out a button
    lcd.DrawBitmap(ui.get_button(0).x, ui.get_button(0).y, resources_calib_bmp);
    ui.get_button(0).action = start_calibration;
    lcd.DrawBitmap(ui.get_button(1).x, ui.get_button(1).y, resources_baubles_bmp);
    ui.get_button(1).action = [](){
        stripey(outer_buffer);
        stripey(inner_buffer);
        motors::set_state(motors::state::spin);
        application_update = std::function<void(void)>();
    };
    ui.get_button(2).action = show_earth;
    lcd.DrawBitmap(ui.get_button(2).x, ui.get_button(2).y, resources_globe_bmp);
 //   ui.get_button(3).text = "Pong";
    lcd.DrawBitmap(ui.get_button(3).x, ui.get_button(3).y, resources_pong_bmp); 
    ui.get_button(3).action = launch_pong;

    lcd.DrawBitmap(ui.get_button(4).x, ui.get_button(4).y, resources_stop_bmp);
    ui.get_button(4).action = []() {
        inner_buffer.clear(ds::GREEN);
        outer_buffer.clear(ds::RED);
        temp_rotation = 0;
        motors::set_state(motors::state::stop);
    };

    lcd.DrawBitmap(ui.get_button(5).x, ui.get_button(5).y, resources_leds_bmp);
    ui.get_button(5).action = init_neopixels;
}

void stripey(render::framebuffer& buffer)
{
    buffer.clear(ds::GREEN);
    for(size_t i=0; i<buffer.n_col(); i+=2)
    {
        buffer.fill_rect(i, 0, 1, buffer.n_row(), ds::RED);
    }
}



int main()
{
    TS_StateTypeDef touch;
    lcd.SetTextColor(LCD_COLOR_WHITE);
    lcd.Clear(BACKGROUND_COLOUR);
    lcd.SetBackColor(BACKGROUND_COLOUR);
    BSP_LCD_SetFont(&Font24);
    // not checking status
    ts.Init(lcd.GetXSize(), lcd.GetYSize());
    prepare_background();
    init_neopixels();
    // 0 means good, non-zero is error code. 
    ds::ring outer(outer_buffer, ds::outer, lcd); 
    ds::ring inner(inner_buffer, ds::inner, lcd);
    stripey(outer_buffer);
    stripey(inner_buffer);
    int outer_col = 0;
    int inner_col = 0;
    outer_buffer.swap();
    inner_buffer.swap();
    setup_main_menu(ui);
    ui.render_all();
    motors::init();
    size_t frame_count = 0;
    size_t max_frames = 8;
    int oi_temp = 0;
    int ii_temp = 0;

    Ticker fast_update;
    Ticker slow_update;
    critical_update = [&]() {
        motors::update();
        // Stops bad position guesses when not spinning from making display glitch
        if(motors::is_spinning())
        {
            oi_temp = (OUTER_WIDTH-1)*motors::position(motors::motor::outer);
            ii_temp = (INNER_WIDTH-1)*motors::position(motors::motor::inner);
            oi_temp = (oi_temp + outer_offset);
            if(oi_temp >= OUTER_WIDTH)
            {
                oi_temp -= OUTER_WIDTH;
            }
            if(temp_rotation != 0) {
                ii_temp = (ii_temp + temp_rotation);
            } else {
                ii_temp = (ii_temp + inner_offset);
            }
            if(ii_temp >= INNER_WIDTH)
            {
                ii_temp -= INNER_WIDTH;
            }
            outer.display(oi_temp);
            inner.display(ii_temp); 
        }
        if(outer_col > oi_temp) {
            outer_buffer.swap();
        }
        if(inner_col > ii_temp) {
            inner_buffer.swap();
        }
        outer_col = oi_temp;
        inner_col = ii_temp;
    };

    fast_update.attach(critical_run,    0.0003f);
    slow_update.attach(application_run, 0.2f);
    while(1)
    {
        if(frame_count == 7)
        {
            ts.GetState(&touch);
            ui.update(touch);
        }
        frame_count++;
        if(frame_count == max_frames) { 
            frame_count = 0;
        }
    }
}
