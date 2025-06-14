namespace Escape {
    /*
        ESC - 0x1b
        CSI - ESC [
        OSC - ESC ]
        SP - 0x20 - space
        ST - ESC 0x5c - \
        DECSET - CSI ? {code} h
        DECRST - CSI ? {code} l
        SGR - CSI {args} m

    
        Save to clipboard
        OSC 52 ; {clipboard} ; {base64-data} ST
        clipboard - empty or s0


        Left-right margins
        DECSET 69
        CSI {left} ; {right} s
        DECRST 69

        Report button press and release
        DECSET 1000
        Report mouse movements while a button is pressed
        DECSET 1002
        Report mouse movements while no button is pressed
        DECSET 1003
        Modern mouse reporting
        DECSET 1006
            Released
            CSI < {params} m 
            All other events
            CSI < {params} M

        Alternate scroll, cursor up and cursor down
        DECSET 1007

        Focus reporting
        DECSET 1004
        DECRST 1004
            Focus
            CSI I
            Lost
            CSI O

        Overline
        on  - SGR 53
        off - SGR 55

        Notifications
        OSC 9 ; {text} ST


        Query Color ? DECRQSS
        OSC 48:2:1:2:3m ESC P $ qm ESC \

        https://vt100.net/docs/vt510-rm/chapter4.html
    */

    /*
    from syscalls of curses

    > TIOCGWINSZ
    > TCSETSW
    > TCGETS

    \33[?1049h\33[22;0;0t\33[1;24r\33(B\33[m\33[4l\33[?7h\33[?25l

    [22;0;0t report xterm title, restore window, restore iconified

    [1;24r r set scrolling region 1;24 set scrolling region

    [B ascii charset

    [m reset

    [4l disable insert mode

    [?7h enable auto wrap

    [?25l hide cursor


    > TCGETS
    > TCSETSW


    \33[39;49m\33]4;0;rgb:00/00/00\33\\\33]4;1;rgb:7F/00/00\33\\\33]4;2;rgb:00/7F/00\33\\\33]4;3;rgb:7F/7F/00\33\\\33]4;4;rgb:00/00/7F\33\\\33]4;5;rgb:7F/00/7F\33\\\33]4;6;rgb:00/7F/7F\33\\\33]4;7;rgb:7F/7F/7F\33\\\33[?1h\33=

    [39;49m set graphics rendition, reset fg and bg to default

    ^[]4;0;rgb:00/00/00^[\      4: set color of palette, 0: palette id, rgb:000000 ST

    [?1h application cursor keys mode, different codes for arrows, home/end

    =  different keypad mode, send codes for keypad (  > to disable   )

    
    > TIOCGWINSZ

    
    \33[39;49m\33[37m\33[40m\33[H\33[2J
    
    [39;49m set graphics rendition, reset fg and bg to default

    [37m fg to white

    [40m bg to black

    [H go home



    > poll({fd=0, events=POLLIN}, 1, 0) = 0 (timeout 0)

    > write more buffers

    > TCSETSW

    */


    struct Cursor {
        // [H
        void cursor_home();
        // [{line};{column}H
        void cursor_move(int x, int y);
        // [{cols}A
        void cursor_up(int cols);
        // [{cols}B
        void cursor_down(int cols);
        // [{cols}C
        void cursor_right(int cols);
        // [{cols}D
        void cursor_left(int cols);
        // [{cols}E
        void cursor_nlcr(int lines);
        // [{cols}F
        void cursor_prev_nlcr(int lines);
        // [{cols}G
        void cursor_column(int col);
        // [6n
        void request_position();
        // M
        void cursor_up_and_scroll();
        // 7
        void save_cursor_position_dec();
        // 8
        void restore_cursor_position_dec();
        // [s
        void save_cursor_position_sco();
        // [u
        void restore_cursor_position_sco();
    };

    struct Erase {
        // [J
        void erase_in_display();
        // [0J
        void erase_cursor_end();
        // [1J
        void erase_cursor_beg();
        // [2J
        void erase_entire_screen();
        // [3J
        void erase_saved_lines();
        // [K
        void erase_in_line();
        // [0K
        void erase_cursor_end_of_line();
        // [1K
        void erase_cursor_start_of_line();
        // [2K
        void erase_line();
    };

    struct TextStyle {
        // [1;34;{...}m
        void set_cell_mode();
        // [0m
        void reset_all_modes();
        // [1m
        void set_bold();
        // [22m
        void reset_bold();
        // [2m
        void set_dim();
        // [22m
        void reset_dim();
        // [3m
        void set_italic();
        // [23m
        void reset_italic();
        // [4m
        void set_underline();
        // [24m
        void reset_underline();
        // [21m
        void set_double_underline();
        // [24m
        void reset_double_underline();
        // [5m
        void set_blinking();
        // [25m
        void reset_blinking();
        // [7m
        void set_reverse();
        // [27m
        void reset_reverse();
        // [8m
        void set_invisible();
        // [28m
        void reset_invisible();
        // [9m
        void set_strikethrough();
        // [29m
        void reset_strikethrough();
    };
    // Bold red foreground
    // [1;31m
    // Dim white foreground with red background
    // [2;37;41m
    struct Colors {
        enum Codes {
            FBLACK = 30,
            BBLACK = 40,
            FRED = 31,
            BRED = 41,
            FGREEN = 32,
            BGREEN = 42,
            FYELLOW = 33,
            BYELLOW = 43,
            FBLUE = 34,
            BBLUE = 44,
            FMAGENTA = 35,
            BMAGENTA = 45,
            FCYAN = 36,
            BCYAN = 46,
            FWHITE = 37,
            BWHITE = 47,
            FDEFAULT = 39,
            BDEFAULT = 49,
        };
    };

    struct BrightColors {
        enum Codes {
            FBLACK = 90,
            BBLACK = 100,
            FRED = 91,
            BRED = 101,
            FGREEN = 92,
            BGREEN = 102,
            FYELLOW = 93,
            BYELLOW = 103,
            FBLUE = 94,
            BBLUE = 104,
            FMAGENTA = 95,
            BMAGENTA = 105,
            FCYAN = 96,
            BCYAN = 106,
            FWHITE = 97,
            BWHITE = 107,            
        };
    };

    // 0-7 standard colors
    // 0-15 high intensity colors
    // 16-231 216 colors 16+36*r+6*g+b (0 <= r, g, b <= 5)
    // 232-255 grayscale dark to light 24 steps
    struct Colors256 {
        // [38;5;{id}m
        void set_foreground();
        // [48;5;{id}m
        void set_background();
    };

    struct ColorsRGB {
        // [38;2;{r};{g};{b}m
        // CSI 38 ; 2 ; {red} ; {green} ; {blue} m
        void set_foreground();
        // [48;2;{r};{g};{b}m
        // CSI 48 ; 2 ; {red} ; {green} ; {blue} m
        void set_background();
    };

    struct Screen {
        enum Codes {
            MONOCHROME_40_25 = 0, // 40x25
            COLOR_40_25 = 1, 
            MONOCHROME_80_25 = 2, // 80x25
            COLOR_80_25 = 3,
            COLOR_4_320_200 = 4, // 4 color 320x200
            MONOCHROME_320_200 = 5, // 320x200
            MONOCHROME_640_200 = 6, // 640x200
            LINE_WRAP = 7,
            COLOR_GRAPHICS_320_200 = 13,
            COLOR_16_640_200 = 14,
            COLOR_2_640_350 = 15,
            COLOR_16_640_350 = 16,
            MONOCHROME_2_640_480 = 17,
            COLOR_16_640_480 = 18,
            COLOR_256_320_200 = 19
        };
        // [={code}h
        void set_mode();
        // [={code}l
        void reset_mode();
    };

    struct NonStandard {
        // [?25l
        void cursor_invisible();
        // [?25h
        void cursor_visible();
        // [?47l
        void restore_screen();
        // [?47h
        void save_screen();
        // [?1000h // [Mbxy upper left (1,1) 0=mb1 1=mb2 2=mb3 3=release 4=shift, 8=meta, 16=control
        void enable_mouse_reporting();
        // [?1000l
        void disable_mouse_reporting();
        // [?1049h
        void enable_alternate_buffer();
        // [?1049l
        void disable_alternate_buffer();
    };

    struct xterm {
        // ]0;{txt}
        void set_icon_window_title();
        // ]1;{txt}
        void set_icon_name();
        // ]2;{txt}
        void set_window_title();
    };

    struct Keyboard {
        // [{code};{string};{...}p
        void set_keybind();

        char *F1 = "0;59",
        *F2 = "0;60",
        *F3 = "0;61",
        *F4 = "0;62",
        *F5 = "0;63",
        *F6 = "0;64";
        
    };

    struct IOCTL {
        // KDGKBMODE
        // K_RAW 0x0
        // K_XLATE 0x1
        // K_MEDIUMRAW 0x2
        // K_UNICODE 0x3
        // K_OFF 0x4

        //KDGETKEYCODE
        // kbkeycode {
        // unsigned int scancode, keycode; }
    };

    // >    numeric keypad
    // =    application keypad
    // ]R   reset palette
    // ]P   set palette nrrggbb n=0-15 color, 0-255 hex
};

struct Key {

};

bool unicode_understanding;
bool utf8_understanding;

int get_utf8_codepoint(char *utf8, int len) {
    if ((utf8[0] & 0x80) == 0)
        return utf8[0];
        
    const bool b7 = 1;
    const bool b6 = (utf8[0] & 0b01000000)>0;
    const bool b5 = (utf8[0] & 0b00100000)>0;
    const bool b4 = (utf8[0] & 0b00010000)>0;
    const bool b3 = (utf8[0] & 0b00001000)>0;

    int cn = 1;
    int ret = 0;
    if (b7 && b6 && !b5) {
        cn = 2;
        ret =  utf8[1];
        ret <<= 6;
        ret |= utf8[0] & 0b111111;
        return ret;
    } else if (b7 && b6 && b5 && !b4) {
        cn = 3;
        ret =  utf8[0] & 0b1111;
        ret <<= 8;
        ret |= utf8[1] & 0b111111;
        ret <<= 6;
        ret |= utf8[2] & 0b111111;
        return ret;
    } else if (b7 && b6 && b5 && b4 && !b3) {
        cn = 4;
        ret =  utf8[0] & 0b111;
        ret <<= 6;
        ret |= utf8[1] & 0b111111;
        ret <<= 6;
        ret |= utf8[2] & 0b111111;
        ret <<= 6;
        ret |= utf8[3] & 0b111111;
        return ret;
    } else 
        return -1;
}


void parse(char *input, int len) {

}