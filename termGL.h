#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#ifndef TERM_GL
# define TERM_GL

/* ESC is a single char used to start ANSI console codes ; see man 4 console_codes */
# define ESC	"\33"

# define SAVE_CURSOR_ORIGIN	write(1, ESC "7", 2)
# define MOVE_CURSOR_TO_ORIGIN	write(1, ESC "8", 2)
# define CLEAR_SCREEN		write(1, ESC "[J", 3)

# define SET_GRAPHIC_MAPPING	write(1, ESC "(0", 3)
# define RESET_GRAPHIC_MAPPING	write(1, ESC "(B", 3)

/* functions to initialize termGL ; call init at the start and destroy at the end.
 * If the program crashes before reaching destroyDisplay, the terminal will still be in VT100 graphic mode ; use the 'reset' command or echo -e "\33(B" or echo -e "\33c" to fix it. */
void	initDisplay(void);
void	destroyDisplay(void);

typedef struct {
	char	*pixels;
	char	*display_buffer;
	unsigned int	size[2];
}		Frame;

/* Frame constructor and destructor */
Frame	createFrame(unsigned int size[2]);
void	destroyFrame(Frame *frame);

void	displayFrame(Frame *frame);
void	clearFrame(Frame *frame);

#endif
