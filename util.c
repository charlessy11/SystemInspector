#include <math.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>

#include "util.h"
#include "logger.h"

int open_path(char *proc_dir, char *path) {
	if (proc_dir == NULL || path == NULL) {
		errno = EINVAL;
		return -1;
	}

	size_t str_size = strlen(proc_dir) + strlen(path) + 2;
	char *full_path = malloc(str_size * sizeof(char));
	if (full_path == NULL) {
		return -1;
	}

	snprintf(full_path, str_size, "%s/%s", proc_dir, path);
	LOG("Opening path: %s\n", full_path);

	int fd = open(full_path, O_RDONLY);
	free(full_path);

	return fd;
}

ssize_t one_lineread(int fd, char *buf, size_t sz, char *delim) {
	ssize_t read_sz = lineread(fd, buf, sz);
	if (read_sz == -1) {
		return -1;
	}

	size_t token_loc = strcspn(buf, delim);
	buf[token_loc] = '\0';

	return read_sz;
}

ssize_t lineread(int fd, char *buf, size_t sz) {
    size_t counter = 0;

    while(counter < sz) {
	    char c;
	    ssize_t read_sz = read(fd, &c, 1);
	    if (read_sz == 0) {
		    //EOF
		    return 0;
	    }
	    else if (read_sz == -1) {
	    	//Error happened
			return -1;
	    }	    
	    else {
	    	buf[counter] = c;
			counter++;

			if (c == '\n') {
				return counter;
			}
	    }
    }    
    return counter;
}

char *next_token(char **str_ptr, const char *delim)
{
    if (*str_ptr == NULL) {
        return NULL;
    }

    size_t tok_start = strspn(*str_ptr, delim);
    size_t tok_end = strcspn(*str_ptr + tok_start, delim);

    /* Zero length token. We must be finished. */
    if (tok_end  == 0) {
        *str_ptr = NULL;
        return NULL;
    }

    /* Take note of the start of the current token. We'll return it later. */
    char *current_ptr = *str_ptr + tok_start;

    /* Shift pointer forward (to the end of the current token) */
    *str_ptr += tok_start + tok_end;

    if (**str_ptr == '\0') {
        /* If the end of the current token is also the end of the string, we
         * must be at the last token. */
        *str_ptr = NULL;
    } else {
        /* Replace the matching delimiter with a NUL character to terminate the
         * token string. */
        **str_ptr = '\0';

        /* Shift forward one character over the newly-placed NUL so that
         * next_pointer now points at the first character of the next token. */
        (*str_ptr)++;
    }

    return current_ptr;
}

void draw_percbar(char *buf, double frac) {
	//percbar has a total of 20 '-'/'#', so we allocate space for that plus allowance
	char percbar[40] = { 0 };
	percbar[0] = '[';
	percbar[21] = ']';

	if (frac <= 0 || isnan(frac)) {
		frac = 0;
	}
	else if (frac > 1 || isinf(frac)) {
		frac = 1;
	}

	int amount = (((round(frac * 100)) / 5 ) + 1);

	int i;
	for (i = 1; i < amount; i++) {
		percbar[i] = '#';
	}
	int j;
	for (j = i; j <= 20; j++) {
		percbar[j] = '-';
	}

	frac = frac * 100;
	buf += snprintf(buf, 40, "%s ", percbar);
	snprintf(buf, 40, "%.1f%%", frac);
}

void uid_to_uname(char *name_buf, uid_t uid) {
	char *path = "/etc/passwd";
	char temp[_POSIX_PATH_MAX];

	int fd = open(path, O_RDONLY);

	bool is_changed = false;
	char *ptr;

	while (true) {
		ssize_t read_sz = one_lineread(fd, temp, _POSIX_PATH_MAX, "\n");
		if (read_sz <= 0) {
			break;
		}

		char name[1000];
        strcpy(name, temp);

        ptr = name;
		sprintf(name, "%s", next_token(&ptr, ":"));
		next_token(&ptr, ":");

		if (atoi(next_token(&ptr, ":")) == uid) {
			sprintf(name_buf, "%s", name);
			is_changed = true;
		}
		if (is_changed == false) {
			sprintf(name_buf, "%d", uid);
		}
	}
	close(fd);
	name_buf[15] = '\0';
}
