#include <ncurses.h>
#include <string.h>

#define KEY_ESCAPE 27
#define KEY_RETURN 10

void render(int num_options, char** options, int selection) {
    for (int i = 0; i < num_options; i++) {
        mvaddstr(i, 0, i == selection ? "> " : "  ");
        addstr(options[i]);
    }
}

int main(int argc, char* argv[]) {
    newterm(NULL, stderr, stdin);
    noecho();
    cbreak();
    keypad(stdscr, true);
    set_escdelay(100);
    use_default_colors();
    // hide cursor
    curs_set(0);
    int selection = 0;

    char** return_on_h = NULL;

    if (argc >= 3 && strcmp("-h", argv[1]) == 0) {
        return_on_h = argv + 2;
        argc -= 2;
        argv += 2;
    }

    int num_options = argc - 1;
    char** options = argv + 1;

    char* output = NULL;

    while (true)
    {
        render(num_options, options, selection);
        int input = getch();
        int exit = 0;
        switch (input) {
        case 'h':
            if (return_on_h) {
                output = *return_on_h;
                exit = 1;
            }
            break;
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
        case 'q':
            exit = 1;
            break;
        case 'l':
        case KEY_RETURN:
            output = options[selection];
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
    if (output) {
        puts(output);
    }
    return 0;
}
