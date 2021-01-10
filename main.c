#include <ncurses.h>
#include <string.h>
#include <locale.h>

#define TERMINAL_HEIGHT LINES
#define KEY_ESCAPE 27
#define KEY_RETURN 10

void render(int num_options, char** items, int y,
            int scroll_from_top, char* header) {
    int i = 0;
    if (header) {
        mvaddstr(0, 2, header);
        i += 2;
    }
    for (int j = i; j < LINES; j++) {
        if (j - i + scroll_from_top < num_options) {
            if (j - i + scroll_from_top == y) {
                mvaddstr(j, 0, "> ");
            } else {
                mvaddstr(j, 0, "  ");
            }
            addstr(items[j - i + scroll_from_top]);
        } else {
            move(j, 0);
        }
        clrtoeol();
    }
}

void setup() {
    setlocale(LC_ALL, "");
    newterm(NULL, stderr, stdin);
    noecho();
    cbreak();
    keypad(stdscr, true);
    set_escdelay(100);
    use_default_colors();
    // hide cursor
    curs_set(0);
}

void update_scroll_from_top(int y, int* scroll_from_top) {
    if (y < *scroll_from_top) {
        *scroll_from_top = y;
    } else if (y > *scroll_from_top + TERMINAL_HEIGHT - 1) {
        *scroll_from_top = y - TERMINAL_HEIGHT + 1;
    }
}

int main(int argc, char* argv[]) {

    setup();

    int y = 0;
    int scroll_from_top = 0;

    char** on_left = NULL;
    char* header = NULL;

    char** args = argv;
    int nargs = argc;

    for (int i = 1; i < nargs; i++) {
        if (strcmp("--on-left", args[i]) == 0) {
            on_left = args + i + 1;
            argc -= 2;
            argv += 2;
        } else if (strcmp("--header", args[i]) == 0) {
            header = args[i+1];
            argc -= 2;
            argv += 2;
        }
    }

    int num_options = argc - 1;
    char** options = argv + 1;

    char* output = NULL;

    while (true)
    {
        update_scroll_from_top(y, &scroll_from_top);
        render(num_options, options, y, scroll_from_top, header);
        int input = getch();
        int exit = 0;
        switch (input) {
        case 'g':
            y = 0;
            break;
        case 'G':
            y = num_options - 1;
            break;
        case 'h':
            if (on_left) {
                output = *on_left;
                exit = 1;
            }
            break;
        case 'k':
            if (y > 0) {
                y -= 1;
            }
            break;
        case 'j':
            if (y < num_options - 1) {
                y += 1;
            }
            break;
        case 'q':
            exit = 1;
            break;
        case 'l':
        case KEY_RETURN:
            output = options[y];
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
