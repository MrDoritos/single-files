#include "../console/advancedConsole.h"
#include "../imgcat/colorMappingPalette.h"

using namespace ColorMappingPalette;

struct Palette {
    std::vector<char_val> charvs;
    std::vector<pix_char> colors;
};

namespace UI {
    int i_charvs = 0, i_colors = 0;

    enum UserState : int {
        INPUT_CHAR,
        INPUT_VALUE,
        INPUT_RED,
        INPUT_GREEN,
        INPUT_BLUE,
        INPUT_END,
    };

    ::Palette palette;
    UserState state{INPUT_CHAR};

    void process_input() {
        int key = NOMOD(console::readKey());

        switch (key) {
            case VK_ESCAPE: exit(0); break;
            case VK_RIGHT:
                state = (UserState)(state + 1);
                if (state >= INPUT_END)
                    state = INPUT_CHAR;
                return;
            case VK_LEFT:
                state = (UserState)(state - 1);
                if (state < 0)
                    state = (UserState)0;
                return;
        }

        switch (state) {
            case INPUT_CHAR:
            case INPUT_VALUE:
                if (key == '\n') {
                    palette.charvs.push_back(char_val()); 
                    i_charvs = palette.charvs.size() - 1;
                }                
                break;
            case INPUT_RED:
            case INPUT_GREEN:
            case INPUT_BLUE:
                if (key == '\n') {
                    palette.colors.push_back(pix_char());
                    i_colors = palette.colors.size() - 1;
                }
                break;
        }
    }
}

int main() {

}