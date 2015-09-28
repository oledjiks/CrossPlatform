#include <stdio.h>
#include "color_print.h"

int main(int argc, char *argv[])
{
    typedef struct{
        enum cp_color color;
        const char * text;
    } out_struct;

    int i = 0;
    out_struct sample[]={
        {CP_FG_RED,"Red Text\n"},
        {CP_FG_BLUE,"Blue Text\n"},
        {CP_FG_GREEN,"Green Text\n"},
        {CP_FG_YELLOW,"Yellow Text\n"},

        {CP_BG_RED,"CP_BG_RED Text\n"},
        {CP_BG_BLUE,"CP_BG_BLUE Text\n"},
        {CP_BG_GREEN,"CP_BG_GREEN Text\n"},
        {CP_FG_GREEN | CP_BG_RED,"CP_FG_WHITE | CP_BG_RED Text\n"}
    };

    cp_state_ptr  cp  = cp_init();

    for (i = 0; i < sizeof(sample)/sizeof(out_struct); ++i)
    {
        cp_print(cp, sample[i].color, sample[i].text);
        printf("Normal Text\n");
    }

    cp_close(cp);
    return 0;
}
