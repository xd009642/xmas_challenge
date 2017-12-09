#include "gui.h"

static constexpr uint32_t WIDTH_OFFSET = 20;
static constexpr uint32_t HEIGHT_OFFSET = 100;

gui::interface::interface(LCD_DISCO_F469NI& screen, size_t n_buttons):
    screen(screen),
    len(n_buttons),
    buttons(nullptr)
{
    if(n_buttons > 0u)
    {
        buttons = new gui::button[n_buttons];
        for(size_t i=0; i<n_buttons; i++)
        {
            buttons[i].x = WIDTH_OFFSET + i*150;
            buttons[i].y = HEIGHT_OFFSET;
            buttons[i].width = 150;
            buttons[i].height = 100;
            buttons[i].border = LCD_COLOR_GREEN;
            buttons[i].text_colour = LCD_COLOR_WHITE;
        }
    }
}

gui::interface::~interface()
{
    if(buttons != nullptr)
    {
        delete [] buttons;
    }
}

void gui::interface::render()
{
    for(size_t i=0; i<len; i++)
    {
        buttons[i].render(screen);
    }
}

gui::button& gui::interface::get_button(const size_t index)
{
    if(index >= len)
    {
        return buttons[len-1];
    }
    else
    {
        return buttons[index];
    }
}

std::function<void()> gui::interface::get_action(const TS_StateTypeDef& pos)
{
    for(size_t i=0; i<len; i++)
    {
        if(buttons[i].poll_event(pos))
            return buttons[i].action;
    }
    return {};
}
