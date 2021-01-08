#include <ncurses.h>
#include <stdio.h>

#define KEY_ESCAPE 27
#define KEY_RETURN 10

void render(int num_options, char** options, int selection) {
    for (int i = 0; i < num_options; i++) {
        mvaddstr(i, 0, i == selection ? "> " : "  ");
        addstr(options[i]);
    }
}

int main(int argc, char* argv[]) {
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, true);
    set_escdelay(100);
    use_default_colors();
    int selection = 0;

    int num_options = argc - 1;
    char** options = argv + 1;

    while (true)
    {
        render(num_options, options, selection);
        int input = getch();
        int exit = 0;
        switch (input) {
        case 'k':
            if (selection > 0) {
                selection -= 1;
            }
            break;
        case 'j':
            if (selection < num_options - 1) {
                selection += 1;
            }
            break;
        case KEY_RETURN:
            exit = 1;
            break;
        default:
            break;
        }
        if (exit) {
            break;
        }
    }
    // refresh display
    refresh();
    // end ncurses mode
    endwin();
    puts(options[selection]);
    return 0;
}
