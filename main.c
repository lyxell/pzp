#include <ncurses.h>
#include <string.h>
#include <locale.h>

#define KEY_ESCAPE 27
#define KEY_RETURN 10

void render(int num_options, char** options, int selection, char* header) {
    int i = 0;
    if (header) {
        mvaddstr(0, 2, header);
        i += 2;
    }
    for (int j = 0; j < num_options && (j + i) < LINES; j++) {
        mvaddstr(j + i, 0, j == selection ? "> " : "  ");
        addstr(options[j]);
    }
}

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "");
    newterm(NULL, stderr, stdin);
    noecho();
    cbreak();
    keypad(stdscr, true);
    set_escdelay(100);
    use_default_colors();
    // hide cursor
    curs_set(0);
    int selection = 0;

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
        render(num_options, options, selection, header);
        int input = getch();
        int exit = 0;
        switch (input) {
        case 'g':
            selection = 0;
            break;
        case 'G':
            selection = num_options - 1;
            break;
        case 'h':
            if (on_left) {
                output = *on_left;
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
