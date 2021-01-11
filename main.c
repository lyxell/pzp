#include <ncurses.h>
#include <string.h>
#include <locale.h>

#define TERMINAL_HEIGHT ((size_t)LINES)
#define KEY_ESCAPE 27
#define KEY_RETURN 10

struct state {
    size_t selected_option;
    int scroll_offset;
    char* header;
    char** options;
    int num_options;
    char* output;
    char* output_on_key_left;
    int is_exiting;
};

void render_state(struct state s) {

    // print header and save the number of lines that was written
    size_t lines_written = 0;
    if (s.header) {
        mvaddstr(0, 2, s.header);
        lines_written += 2;
    }

    // print the options, only print as many as will fit the screen
    for (int i = 0; i < TERMINAL_HEIGHT - lines_written; i++) {
        if (i + s.scroll_offset < s.num_options) {
            if (i + s.scroll_offset == s.selected_option) {
                mvaddstr(i + lines_written, 0, "- ");
            } else {
                mvaddstr(i + lines_written, 0, "  ");
            }
            addstr(s.options[i + s.scroll_offset]);
        } else {
            move(i + lines_written, 0);
        }
        clrtoeol();
    }

}

void setup_terminal() {
    setlocale(LC_ALL, "");
    newterm(NULL, stderr, stdin);
    noecho();
    cbreak();
    keypad(stdscr, true);
    set_escdelay(100);
    use_default_colors();
    curs_set(0);
}

void teardown_terminal() {
    refresh();
    endwin();
}

struct state handle_input(struct state s, int input) {
    switch (input) {
    case 'g':
        s.selected_option = 0;
        break;
    case 'G':
        s.selected_option = s.num_options - 1;
        break;
    case 'h':
        if (!s.output_on_key_left) break;
        s.output = s.output_on_key_left;
        s.is_exiting = 1;
        break;
    case 'k':
        if (s.selected_option == 0) break;
        s.selected_option -= 1;
        break;
    case 'j':
        if (s.selected_option == s.num_options - 1) break;
        s.selected_option += 1;
        break;
    case 'q':
        s.is_exiting = 1;
        break;
    case 'l':
    case KEY_RETURN:
        s.output = s.options[s.selected_option];
        s.is_exiting = 1;
        break;
    case KEY_PPAGE:
        if (TERMINAL_HEIGHT > s.scroll_offset) {
            s.scroll_offset = 0;
        } else {
            s.scroll_offset -= TERMINAL_HEIGHT;
        }
        if (TERMINAL_HEIGHT > s.selected_option) {
            s.selected_option = 0;
        } else {
            s.selected_option -= TERMINAL_HEIGHT;
        }
        break;
    case KEY_NPAGE:
        if (s.scroll_offset + TERMINAL_HEIGHT > s.num_options - 1) {
            s.scroll_offset = s.num_options - 1;
        } else {
            s.scroll_offset += TERMINAL_HEIGHT;
        }
        if (s.selected_option + TERMINAL_HEIGHT > s.num_options - 1) {
            s.selected_option = s.num_options - 1;
        } else {
            s.selected_option += TERMINAL_HEIGHT;
        }
        break;
    default:
        break;
    }
    return s;
}

struct state update_scroll_offset(struct state s) {
    if (s.selected_option < s.scroll_offset) {
        s.scroll_offset = s.selected_option;
    } else if (s.selected_option > s.scroll_offset + TERMINAL_HEIGHT - 1) {
        s.scroll_offset = s.selected_option - TERMINAL_HEIGHT + 1;
    }
    return s;
}

int main(int argc, char* argv[]) {

    setup_terminal();

    struct state s = {0};
    s.options = argv + 1;
    s.num_options = argc - 1;

    // parse command line options
    for (int i = 1; i < argc; i++) {
        if (strcmp("--on-left", argv[i]) == 0) {
            s.output_on_key_left = argv[i + 1];
            s.options += 2;
            s.num_options -= 2;
        } else if (strcmp("--header", argv[i]) == 0) {
            s.header = argv[i+1];
            s.options += 2;
            s.num_options -= 2;
        }
    }
    
    // render loop
    while (true) {
        s = update_scroll_offset(s);
        render_state(s);
        s = handle_input(s, getch());
        if (s.is_exiting) {
            break;
        }
    }

    teardown_terminal();

    // print output if any
    if (s.output) {
        puts(s.output);
    }

    return 0;

}

