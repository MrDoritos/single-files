//#include "../console/console.h"

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <signal.h>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <atomic>
#include <string>
#include <vector>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <poll.h>
#include <thread>
#include <mutex>
#include <condition_variable>

/*

^[[?1049h       buffer
^[[22;0;0t      push title
^[[1;10r        set top and bottom margins
^[(B            set ascii charset
^[[m            reset graphic rendition
^[[4l           reset 4 insert mode
^[[?7h          set ?7 wraparound mode
^[[?25l         reset ?25 cursor visible
^[[39;49m       default foreground ; default background
^[]4;0;rgb:00/00/00^[\  change/read palette color
^[]4;1;rgb:7F/00/00^[\
^[]4;2;rgb:00/7F/00^[\
^[]4;3;rgb:7F/7F/00^[\
^[]4;4;rgb:00/00/7F^[\
^[]4;5;rgb:7F/00/7F^[\
^[]4;6;rgb:00/7F/7F^[\
^[]4;7;rgb:7F/7F/7F^[\
^[[?1h          set cursor key format (decckm)
^[=             set application keypad
^[[39;49m       default foreground ; default background
^[[37m          set foreground color
^[[40m          set background color
^[[H            cursor to home position
^[[2J           erase display complete
^[[39;49m       -
^[[10d          cursor vertical position absolute
^[[K            erase line
^[[39;49m       -
^[]104          reset palette colors
^G              bell
^[[10;1H        -
^[[?12l         reset cursor blink
^[[?25h         set ?25 cursor visible
^[[?1049l       reset buffer
^[[23;0;0t      push title
^M              newline
^[[?1l          reset cursor key format (decckm)
^[>             reset application keypad

*/

namespace codes {
    static const char *ESCAPE               = "\033";
    static const char *PUSH_TERMINAL_TITLE  = "\033[22;0;0t";
    static const char *POP_TERMINAL_TITLE   = "\033[23;0;0t";
    static const char *ENABLE_KEYPAD        = "\033=";
    static const char *DISABLE_KEYPAD       = "\033>";
    static const char *ASCII_CHARSET        = "\033(B";
}

namespace envs {
    static const char *COLORTERM            = "COLORTERM";
    static const char *COLORFGBG            = "COLORFGBG";
}

struct console {
    using color_t = unsigned char;

    static inline FILE *get_output_file() {
        return stdout;
    }

    static inline FILE *get_input_file() {
        return stdin;
    }

    static inline const char *get_color_env() {
        const char *var = getenv(envs::COLORTERM);
        if (!var)
            var = getenv(envs::COLORFGBG);
        if (!var)
            var = "";
        return var;
    }

    static inline const char read_raw() {
        char ret;
        fread(&ret, 1, 1, get_input_file());
        //read(fileno(get_input_file()), &ret, 1);
        return ret;
    }

    static inline int read_key() {
        return fgetc_unlocked(get_input_file());
    }

    static inline const void set_cursor(int x, int y) {
        fprintf(get_output_file(), "\033[%i;%iH", x, y);
    }

    static inline const void set_color(int fg, int bg) {
        fprintf(get_output_file(), "\033[%i;%im", fg, bg);
    }

    static inline const void set_color() {
        set_color(39, 49);
    }
    
    enum _Modes {
        INSERT = 4,
    } static Modes;

    enum _Modesx {
        KEY_FORMAT = 1,
        WRAP = 7,
        CURSOR_BLINK = 12,
        CURSOR_VISIBLE = 25,
        REPORT_MOUSE_MOVEMENT = 1003,
        REPORT_FOCUS_CHANGE = 1004,
        BUFFER = 1049
    } static Modesx;

    enum _Modesxx {
        ASCII_CHARSET,
        KEYPAD,
        UNBUFFERED,
        LINEBUFFERED
    } static Modesxx;

    const static inline void _set_mode(const int &code, const char &hl, const char *sp = "") {
        fprintf(get_output_file(), "\033[%s%i%c", sp, code, hl); 
    }

    template<_Modesxx v>
    const static inline void set_mode();

    template<_Modesxx v>
    const static inline void unset_mode();

    const static inline void set_mode(const _Modes &mode) {
        _set_mode(mode, 'h');
    }

    const static inline void set_mode(const _Modesx &mode) {
        _set_mode(mode, 'h', "?");
    }

    const static inline void unset_mode(const _Modes &mode) {
        _set_mode(mode, 'l');
    }

    const static inline void unset_mode(const _Modesx &mode) {
        _set_mode(mode, 'l', "?");
    }

    const static inline void write(const char *text, const color_t &color = 0) {
        const int len = strlen(text);
        fwrite(text, 1, len, get_output_file());
        //fflush(get_output_file());
    }

    const static inline void write(const char &character, const color_t &color = 0) {
        fwrite(&character, 1, 1, get_output_file());
        //fflush(get_output_file());
    }
};

struct async_console : public console {
    static pollfd pfd;
    static int flags;
    static bool polling;
    static std::thread poll_thread;
    static std::mutex input_mutex;
    static std::condition_variable input_ready_cv;

    static void init() {
        pfd.fd = fileno(get_input_file());
        pfd.events = POLLIN;
        flags = fcntl(fileno(get_input_file()), F_GETFL, 0);
        polling = false;
        fcntl(fileno(get_input_file()), F_SETFL, flags | O_NONBLOCK);
        start_polling();
    }

    static void stop_polling() {
        if (!polling)
            return;

        polling = false;

        poll_thread.join();
    }

    static void start_polling() {
        if (polling)
            return;

        polling = true;

        poll_thread = std::thread(do_polling);
    }

    static void wait_for_input() {
        std::unique_lock<std::mutex> lock(input_mutex);
        input_ready_cv.wait(lock);
    }

    static void input_handler(void *buffer, ssize_t bytes_available) {
        input_ready_cv.notify_all();
        fwrite(buffer, 1, bytes_available, get_output_file());
        fflush(get_output_file());
    }

    static void do_polling() {
        const int buf_size = 256;
        char buf[buf_size];
        while (polling) {
            ssize_t bytes_read = read(fileno(get_input_file()), buf, buf_size);

            if (bytes_read == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    continue;
                if (errno == EINTR) {
                    polling = false;
                    break;
                }
            } else
            if (bytes_read == 0) {
                polling = false;
                break;
            } else {
                input_handler(buf, bytes_read);
            }
        }
        input_ready_cv.notify_all();
    }

    static void destroy() {
        stop_polling();
        fcntl(fileno(get_input_file()), F_SETFL, flags);
    }
};

pollfd async_console::pfd;
int async_console::flags;
bool async_console::polling;
std::thread async_console::poll_thread;
std::mutex async_console::input_mutex;
std::condition_variable async_console::input_ready_cv;


template<>
const inline void console::set_mode<console::_Modesxx::ASCII_CHARSET>() {
    console::write(codes::ASCII_CHARSET);
}

template<>
const inline void console::set_mode<console::_Modesxx::KEYPAD>() {
    console::write(codes::ENABLE_KEYPAD);
}

template<>
const inline void console::unset_mode<console::_Modesxx::ASCII_CHARSET>() {
    
}

template<>
const inline void console::unset_mode<console::_Modesxx::KEYPAD>() {
    console::write(codes::DISABLE_KEYPAD);
}

template<> const inline void console::set_mode<console::_Modesxx::UNBUFFERED>() {
    termios s;
    tcgetattr(fileno(get_input_file()), &s);
    s.c_lflag &= (~ICANON & ~ECHO);
    tcsetattr(fileno(get_input_file()), TCSANOW, &s);
}

template<> const inline void console::set_mode<console::_Modesxx::LINEBUFFERED>() {
    termios s;
    tcgetattr(fileno(get_input_file()), &s);
    s.c_lflag |= ICANON | ECHO;
    tcsetattr(fileno(get_input_file()), TCSANOW, &s);
}

console::_Modes console::Modes;
console::_Modesx console::Modesx;
console::_Modesxx console::Modesxx;

bool stop = false;

void handle_signal(int signal) {
    stop = true;
}

void program_exit() {
    async_console::destroy();
    console::unset_mode(console::BUFFER);
    console::set_mode(console::CURSOR_VISIBLE);
    console::unset_mode(console::WRAP);
    console::unset_mode(console::REPORT_MOUSE_MOVEMENT);
    console::write("bye-bye!\n");
    console::set_mode<console::LINEBUFFERED>();
}

int main() {
    console::set_mode<console::UNBUFFERED>();
    console::set_mode(console::BUFFER);
    console::unset_mode(console::CURSOR_VISIBLE);
    console::unset_mode(console::INSERT);
    console::set_mode(console::WRAP);
    console::set_mode(console::REPORT_MOUSE_MOVEMENT);
    console::set_mode<console::ASCII_CHARSET>();
    console::set_mode<console::KEYPAD>();
    console::set_color(35, 49);
    async_console::init();

    signal(SIGTERM, handle_signal);
    signal(SIGQUIT, handle_signal);
    signal(SIGKILL, handle_signal);
    signal(SIGILL, handle_signal);
    signal(SIGABRT, handle_signal);
    signal(SIGSEGV, handle_signal);
    signal(SIGINT, handle_signal);

    console::write(console::get_color_env());
    while (!stop) {
        //console::write(console::read_raw());
        //console::write(std::to_string(console::read_key()).c_str());
        console::write(" ");
        console::write("\u2591");
        console::write("\u2592");
        console::write("\u2593");
        console::write("\u2588");
        async_console::wait_for_input();
    }
    program_exit();
    return 0;
}