#ifndef __COLOR_PRINT_HPP__
#define __COLOR_PRINT_HPP__

#include <iostream>
#include <string>

#if defined __WIN32 || defined __WIN64 || defined WIN32 || defined WIN64
#ifndef WINDOWS
#define WINDOWS
#endif
// other platform
#endif

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef WINDOWS
#include <windows.h>
#else
#define TRUE true
#endif

/* Colors */
enum  cp_color {
#ifdef WINDOWS
    CP_FG_BLACK         = 0x0000,
    CP_FG_BLUE          = 0x0001,
    CP_FG_GREEN         = 0x0002,
    CP_FG_CYAN          = 0x0003,
    CP_FG_RED           = 0x0004,
    CP_FG_MAGENTA       = 0x0005,
    CP_FG_BROWN         = 0x0006,
    CP_FG_LIGHTGRAY     = 0x0007,
    CP_FG_GRAY          = 0x0008,
    CP_FG_LIGHTBLUE     = 0x0009,
    CP_FG_LIGHTGREEN    = 0x000a,
    CP_FG_LIGHTCYAN     = 0x000b,
    CP_FG_LIGHTRED      = 0x000c,
    CP_FG_LIGHTMAGENTA  = 0x000d,
    CP_FG_YELLOW        = 0x000e,
    CP_FG_WHITE         = 0x000f,

    CP_BG_BLUE          = BACKGROUND_BLUE,
    CP_BG_GREEN         = BACKGROUND_GREEN,
    CP_BG_RED           = BACKGROUND_RED,
    CP_BG_GRAY          = BACKGROUND_INTENSITY,
#else  // LINUX
    CP_FG_BLACK         = 0x0001,
    CP_FG_RED           = 0x0002,
    CP_FG_GREEN         = 0x0003,
    CP_FG_YELLOW        = 0x0004,
    CP_FG_BLUE          = 0x0005,
    CP_FG_MAGENTA       = 0x0006,
    CP_FG_CYAN          = 0x0007,
    CP_FG_WHITE         = 0x0008,
    CP_FG_NULL          = 0x000f,

    CP_BG_BLACK         = 0x0010,
    CP_BG_RED           = 0x0020,
    CP_BG_GREEN         = 0x0030,
    CP_BG_YELLOW        = 0x0040,
    CP_BG_BLUE          = 0x0050,
    CP_BG_MAGENTA       = 0x0060,
    CP_BG_GYAN          = 0x0070,
    CP_BG_WHITE         = 0x0080,
    CP_BG_NULL          = 0x00f0,
#endif
    CP_DEF              = 0x00ff,
};

class ColorPrint
{
  private:
    /* Color-Print State */
    struct cp_state
    {
        short    default_color;
        short    current_color;
#ifdef WINDOWS
        HANDLE   std_output;
#else
        FILE*    std_output;
#endif
    };
    typedef struct cp_state* cp_state_ptr;

    /* Module Functions */
    cp_state_ptr cp_init();
    int  cp_print(cp_state_ptr, enum cp_color, const char * ) const;
    void cp_reset(cp_state_ptr ) const;
    void cp_close(cp_state_ptr ) const;

  public:
    ColorPrint();
    ColorPrint(cp_color color, const char* text);
    ColorPrint(cp_color color, const std::string text);
    virtual ~ColorPrint();

    friend int cp_apply(cp_state_ptr cp);
    friend std::ostream& operator<<(std::ostream&, const ColorPrint&);

  private:
    static unsigned int _count;
    cp_state_ptr _cp;
    cp_color _color;
    std::string _text;
};

unsigned int ColorPrint::_count = 0;

ColorPrint::ColorPrint() : _color(CP_DEF), _text("")
{
    if (((_count)++) == 0)
        _cp = cp_init();
}

ColorPrint::ColorPrint(cp_color color, const char* text) : _color(color), _text(text)
{
    if (((_count)++) == 0)
        _cp = cp_init();
}

ColorPrint::ColorPrint(cp_color color, const std::string text) : _color(color), _text(text)
{
    if (((_count)++) == 0)
        _cp = cp_init();
}

ColorPrint::~ColorPrint()
{
    if ((--(_count)) == 0)
        cp_close(_cp);
}

std::ostream& operator<<(std::ostream& os, const ColorPrint& c)
{
    c.cp_print(c._cp, c._color, c._text.c_str());
    return os;
}

int cp_apply(ColorPrint::cp_state_ptr cp)
{
#ifdef WINDOWS
    return SetConsoleTextAttribute(cp->std_output, cp->current_color) == TRUE ? 0:(-1);
#else
    if (cp->current_color == CP_DEF)
        return (fprintf(cp->std_output, "\e[0m") > 0) ? 0:(-1);

    int cp_fg = ((cp->current_color & 0x0f) < 0x0f) ? (cp->current_color & 0x0f)+29 : 0;
    int cp_bg = (((cp->current_color >> 4) & 0x0f) < 0x0f) ? ((cp->current_color >> 4) & 0x0f)+39 : 0;
    if (cp_fg > 0)
        if (fprintf(cp->std_output, "\e[%dm\e[%dm", cp_fg, cp_bg) < 0)
            return (-1);
    if (cp_bg > 0)
        if (fprintf(cp->std_output, "\e[%dm", cp_fg) < 0)
            return (-1);
#endif
}

ColorPrint::cp_state_ptr ColorPrint::cp_init()
{
    cp_state_ptr cp;
    cp = (cp_state_ptr)malloc(sizeof(struct cp_state));
    assert(cp);

#ifdef WINDOWS
    HANDLE                     std_output;
    CONSOLE_SCREEN_BUFFER_INFO screen_buff;

    std_output = GetStdHandle(STD_OUTPUT_HANDLE);

    if (std_output == INVALID_HANDLE_VALUE)
        return 0;

    if(!GetConsoleScreenBufferInfo(std_output, &screen_buff))
        return 0;

    cp->std_output = std_output;
    cp->default_color = cp->current_color = screen_buff.wAttributes;
#else
    cp->std_output = stdout;
    cp->default_color = cp->current_color = CP_DEF;
#endif

    return cp;
}

int ColorPrint::cp_print(cp_state_ptr cp, enum cp_color color, const char * text) const
{
    int ret;
    assert(cp);

    cp->current_color = color;
    cp_apply(cp);

    ret = printf("%s", text);

    cp_reset(cp);

    return ret;
}

void ColorPrint::cp_reset(cp_state_ptr cp) const
{
    assert(cp);

    cp->current_color = cp->default_color ;
    cp_apply(cp);
}

void ColorPrint::cp_close(cp_state_ptr cp) const
{
    assert(cp);

    cp_reset(cp);

    if (cp)
    {
        free(cp);
        cp = NULL;
    }
}

#endif /* __COLOR_PRINT_HPP__ */
