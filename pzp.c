#include <ncurses.h>
#include <string.h>
#include <locale.h>

#define TERMINAL_HEIGHT ((size_t)LINES)
#define KEY_ESCAPE 27
#define KEY_RETURN 10

struct state {
    size_t selected_option_index;
    int scroll_offset;
    char* header;
    char** options;
    int num_options;
    char* output;
    char* output_on_key_left;
    int is_exiting;
    int search_mode;
    char search_string[32];
};

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
            if (i + s.scroll_offset == s.selected_option_index) {
                mvaddstr(i + lines_written, 0, "> ");
            } else {
                mvaddstr(i + lines_written, 0, "  ");
            }
            addstr(s.options[i + s.scroll_offset]);
        } else {
            move(i + lines_written, 0);
        }
        clrtoeol();
    }

    if (s.search_mode) {
        move(TERMINAL_HEIGHT - 1, 0);
        addstr("Search: ");
        addstr(s.search_string);
    }

}

struct state handle_input(struct state s, int input) {
    if (s.search_mode) {
        int len = strlen(s.search_string);
        switch (input) {
        case KEY_ESCAPE:
            s.search_mode = 0;
            break;
        case KEY_BACKSPACE:
            if (len > 0) {
                s.search_string[len-1] = '\0';
            }
            break;
        default:
            s.search_string[len] = input;
            break;
        }
    } else {
        switch (input) {
        case 'g':
            s.selected_option_index = 0;
            break;
        case 'G':
            s.selected_option_index = s.num_options - 1;
            break;
        case 'h':
            if (!s.output_on_key_left) break;
            s.output = s.output_on_key_left;
            s.is_exiting = 1;
            break;
        case 'k':
            if (s.selected_option_index == 0) break;
            s.selected_option_index -= 1;
            break;
        case 'j':
            if (s.selected_option_index == s.num_options - 1) break;
            s.selected_option_index += 1;
            break;
        case 'q':
            s.is_exiting = 1;
            break;
        case 'l':
        case KEY_RETURN:
            s.output = s.options[s.selected_option_index];
            s.is_exiting = 1;
            break;
        case KEY_PPAGE:
            if (TERMINAL_HEIGHT > s.scroll_offset) {
                s.scroll_offset = 0;
            } else {
                s.scroll_offset -= TERMINAL_HEIGHT;
            }
            if (TERMINAL_HEIGHT > s.selected_option_index) {
                s.selected_option_index = 0;
            } else {
                s.selected_option_index -= TERMINAL_HEIGHT;
            }
            break;
        case KEY_NPAGE:
            if (s.scroll_offset + TERMINAL_HEIGHT > s.num_options - 1) {
                s.scroll_offset = s.num_options - 1;
            } else {
                s.scroll_offset += TERMINAL_HEIGHT;
            }
            if (s.selected_option_index + TERMINAL_HEIGHT > s.num_options - 1) {
                s.selected_option_index = s.num_options - 1;
            } else {
                s.selected_option_index += TERMINAL_HEIGHT;
            }
            break;
        case '/':
            s.search_mode = 1;
            break;
        default:
            break;
        }
    }
    return s;
}

struct state update_scroll_offset(struct state s) {
    int header_height = s.header ? 2 : 0;
    int last_rendered_index = s.scroll_offset + TERMINAL_HEIGHT
                                              - 1 - header_height;
    if (s.selected_option_index < s.scroll_offset) {
        s.scroll_offset = s.selected_option_index;
    } else if (s.selected_option_index > last_rendered_index) {
        s.scroll_offset = s.selected_option_index - TERMINAL_HEIGHT
                                                  + 1 + header_height;
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
        // only works if --start-at is last flag
        } else if (strcmp("--start-at", argv[i]) == 0) {
            s.options += 2;
            s.num_options -= 2;
            for (int j = 0; j < s.num_options; j++) {
                if (strcmp(s.options[j], argv[i+1]) == 0) {
                    s.selected_option_index = j;
                }
            }
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

