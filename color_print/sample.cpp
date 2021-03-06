#include "color_print.hpp"
#include <iostream>

int main(int argc, char *argv[])
{
    typedef struct{
        enum cp_color color;
        const char * text;
    } out_struct;

    unsigned int i = 0;
    out_struct sample[]={
        {CP_FG_RED,"Red Text\n"},
        {CP_FG_BLUE,"Blue Text\n"},
        {CP_FG_GREEN,"Green Text\n"},
        {CP_FG_YELLOW,"Yellow Text\n"},

        {CP_BG_RED,"CP_BG_RED Text\n"},
        {CP_BG_BLUE,"CP_BG_BLUE Text\n"},
        {CP_BG_GREEN,"CP_BG_GREEN Text\n"},
        // {CP_FG_GREEN | CP_BG_RED,"CP_FG_WHITE | CP_BG_RED Text\n"}
    };

    for (i = 0; i < sizeof(sample)/sizeof(out_struct); ++i)
    {
        std::cout << ColorPrint(sample[i].color, sample[i].text)
                  << "-------------------------------" << std::endl;
        printf("Normal Text\n");
    }

    return 0;
}
