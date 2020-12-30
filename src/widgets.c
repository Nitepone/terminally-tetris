#include "widgets.h"
#include "generic.h"
#include "player.h"
#include <form.h>
#include <menu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define CTRLD 4

void print_in_middle(WINDOW *win, int starty, int startx, int width,
                     char *string, chtype color);

// Not really a proper strip function. This terminates a string as soon as it
// sees a space.
static void strip(char *text) {
	char *c = text;
	while (*c != ' ' && *c != 0)
		c += 1;
	*c = 0;
}

void ttviz_entry(char *username, char *label, int max_length) {
	int label_len = strlen(label);

	FIELD *field[2];
	FORM *my_form;
	int ch;

	/* Initialize the fields */
	field[0] = new_field(1, 10, 4, label_len + 11, 0, 0);
	field[1] = NULL;

	/* Set field options */
	set_field_back(field[0],
	               A_UNDERLINE); /* Print a line for the option 	*/
	field_opts_off(field[0],
	               O_AUTOSKIP); /* Don't go to next field when this */
	                            /* Field is filled up 		*/

	/* Create the form and post it */
	my_form = new_form(field);
	post_form(my_form);
	refresh();

	mvprintw(4, 10, label);
	refresh();

	/* Loop through to get user requests */
	while ((ch = getch()) != '\n') {
		switch (ch) {
		case KEY_DOWN:
			/* Go to next field */
			form_driver(my_form, REQ_NEXT_FIELD);
			/* Go to the end of the present buffer */
			/* Leaves nicely at the last character */
			form_driver(my_form, REQ_END_LINE);
			break;
		case KEY_UP:
			/* Go to previous field */
			form_driver(my_form, REQ_PREV_FIELD);
			form_driver(my_form, REQ_END_LINE);
			break;
		default:
			/* If this is a normal character, it gets */
			/* Printed				  */
			form_driver(my_form, ch);
			break;
		}
	}

	// call to form driver is necessary before we can read from the field
	// buffer
	form_driver(my_form, REQ_VALIDATION);
	char *result = field_buffer(field[0], 0);
	strncpy(username, result, max_length);
	// it seems like the form field always has extra spaces, so get rid of
	// those
	strip(username);

	/* Un post form and free the memory */
	unpost_form(my_form);
	free_form(my_form);
	free_field(field[0]);
	free_field(field[1]);

	endwin();
}

struct ttetris_widget_selection {
	/* options is a reference to the options provided by the caller */
	char **options;
	int num_options;
	int num_selected;
	int *indices;
};

void selection_destroy(WidgetSelection *selection) {
	free(selection->indices);
	free(selection);
}

/**
 * Select from a given number of options
 */
struct ttetris_widget_selection *ttviz_select(char **options, int num_options,
                                              char *desc,
                                              int is_single_selection) {
	int win_width = 20;
	ITEM **my_items;
	int c, i;
	MENU *my_menu;

	WINDOW *my_menu_win;
	my_items = (ITEM **)calloc(num_options, sizeof(ITEM *));
	for (i = 0; i < num_options; ++i)
		my_items[i] = new_item(/*name*/ options[i], /*desc*/ NULL);

	/* Crate menu */
	my_menu = new_menu((ITEM **)my_items);

	/* Create the window to be associated with the menu */
	my_menu_win = newwin(10, win_width, 4, 4);
	keypad(my_menu_win, TRUE);

	/* Set main window and sub window */
	set_menu_win(my_menu, my_menu_win);
	WINDOW *child_win = derwin(my_menu_win, 6, win_width - 2, 3, 1);
	set_menu_sub(my_menu, child_win);

	// Set the menu mark. This is shown next to BOTH selected items and the
	// current item, which is kind of confusing. Make sure this is at least
	// two characters, which will slightly distinguish selected items from
	// the current item.
	// TODO Find a better multi-select menu.
	set_menu_mark(my_menu, "---");

	if (!is_single_selection) {
		mvprintw(1, 1, "Use <SPACE> to select or unselect an item");
		mvprintw(2, 1, "Use <ENTER> to confirm selection");
		menu_opts_off(my_menu, O_ONEVALUE);
	}

	/* Print a border around the main window and print a title */
	box(my_menu_win, 0, 0);
	print_in_middle(my_menu_win, 1, 0, win_width, desc, COLOR_PAIR(1));
	mvwaddch(my_menu_win, 2, 0, ACS_LTEE);
	mvwhline(my_menu_win, 2, 1, ACS_HLINE, win_width - 2);
	mvwaddch(my_menu_win, 2, win_width - 1, ACS_RTEE);
	refresh();

	/* Post the menu */
	post_menu(my_menu);
	wrefresh(my_menu_win);

	while ((c = wgetch(my_menu_win)) != '\n') {

		switch (c) {
		case KEY_DOWN:
			menu_driver(my_menu, REQ_DOWN_ITEM);
			break;
		case KEY_UP:
			menu_driver(my_menu, REQ_UP_ITEM);
			break;
		case ' ':
			// space is not bound for single selection menus
			if (!is_single_selection)
				menu_driver(my_menu, REQ_TOGGLE_ITEM);
			break;
		}
		wrefresh(my_menu_win);
	}

	WidgetSelection *w_selection = malloc(sizeof(WidgetSelection));
	w_selection->options = options;
	w_selection->num_options = num_options;

	ITEM **items = menu_items(my_menu);

	if (is_single_selection) {
		ITEM *cur = current_item(my_menu);
		w_selection->num_selected = 1;
		w_selection->indices = malloc(sizeof(int));
		w_selection->indices[0] = item_index(cur);
	} else {
		// count the number of items
		w_selection->num_selected = 0;
		for (i = 0; i < item_count(my_menu); ++i)
			if (item_value(items[i]) == TRUE)
				w_selection->num_selected += 1;

		// store the selected indices
		w_selection->indices =
		    calloc(sizeof(int), w_selection->num_selected);
		int j = 0;
		for (i = 0; i < item_count(my_menu); ++i)
			if (item_value(items[i]) == TRUE)
				w_selection->indices[j++] = i;
	}

	/* Unpost and free all the memory taken up */
	unpost_menu(my_menu);
	free_menu(my_menu);
	for (i = 0; i < num_options; ++i)
		free_item(my_items[i]);

	// clear the window border
	wborder(my_menu_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');

	// run window erase and refresh so that the window will be blank
	werase(my_menu_win);
	wrefresh(my_menu_win);

	// delete the window
	delwin(my_menu_win);

	free(my_items);

	return w_selection;
}

int selection_to_index(WidgetSelection *selection) {
	return selection->indices[0];
}

StringArray *selection_to_string_array(WidgetSelection *selection) {
	StringArray *arr =
	    string_array_create(selection->num_selected, PLAYER_NAME_MAX_CHARS);

	for (int i = 0; i < selection->num_selected; ++i)
		string_array_set_item(arr, selection->indices[i],
		                      selection->options[i]);

	return arr;
}

void print_in_middle(WINDOW *win, int starty, int startx, int width,
                     char *string, chtype color) {
	int length, x, y;
	float temp;

	if (win == NULL)
		win = stdscr;
	getyx(win, y, x);
	if (startx != 0)
		x = startx;
	if (starty != 0)
		y = starty;
	if (width == 0)
		width = 80;

	length = strlen(string);
	temp = (width - length) / 2;
	x = startx + (int)temp;
	wattron(win, color);
	mvwprintw(win, y, x, "%s", string);
	wattroff(win, color);
	refresh();
}
