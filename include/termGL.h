#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#ifndef TERM_GL
# define TERM_GL

/* ESC is a single char used to start ANSI console codes ; see man 4 console_codes */
# define ESC	"\33"

# define SCROLL_UP		ESC "[2J"

# define CURSOR_TO_ORIGIN	ESC "[H"
# define ERASE_DISPLAY		ESC "[J"
# define SET_GRAPHIC_MAPPING	ESC "(0"
# define RESET_GRAPHIC_MAPPING	ESC "(B"

# define DISPLAY_OVERHEAD_START		CURSOR_TO_ORIGIN ERASE_DISPLAY SET_GRAPHIC_MAPPING

# define DISPLAY_OVERHEAD_START_SIZE	\
	(sizeof(CURSOR_TO_ORIGIN) - 1\
	+ sizeof(ERASE_DISPLAY) - 1\
	+ sizeof(SET_GRAPHIC_MAPPING) - 1)

# define DISPLAY_OVERHEAD_END		RESET_GRAPHIC_MAPPING

# define DISPLAY_OVERHEAD_END_SIZE	\
	(sizeof(RESET_GRAPHIC_MAPPING) - 1)

# define DISPLAY_OVERHEAD_SIZE	(DISPLAY_OVERHEAD_START_SIZE + DISPLAY_OVERHEAD_END_SIZE)

void	initDisplay(void);

typedef struct {
	char	* const pixels;
	const size_t	size[2];
	char	* const display_buffer;
	const size_t	display_buffer_size;
}		Frame;

/* Frame constructor and destructor */
Frame	createFrame(const size_t size[2]);
void	destroyFrame(Frame *frame);

void	displayFrame(Frame *frame);
void	clearFrame(Frame *frame);

/* FORCE_MONOSPACE will stretch every Frame's pixel by 2 horizontally so pixels appear as wide as they are tall.
 * This will cause issues with pattern using characters other than PIXEL and HLINE_* since they will appear repeated
 * Off by default. */
# ifndef FORCE_MONOSPACE
#  define FORCE_MONOSPACE	0
# endif

typedef struct {
	char	* const content;
	const size_t size[2];
}		Image;

Image	createImage(const size_t size[2], const char *str);
void	destroyImage(Image *image);

void	putImageToFrame(const Image *image, Frame *frame, const size_t pos[2]);

/* VT100 graphic mappings */
# define PIXEL		'a'
# define DOT		'~'
# define DIAMOND	'`'

# define ULEFT_CORNER	'l'
# define DLEFT_CORNER	'm'
# define URIGHT_CORNER	'i'
# define LRIGHT_CORNER	'j'

# define HLINE_1	's'
# define HLINE_2	'r'
# define HLINE_3	'q'
# define HLINE_4	'p'
# define HLINE_5	'o'

# define VLINE		'x'

# define CROSSING_LINE	'n'

# define LEFT_T		't'
# define RIGHT_T	'u'
# define BOTTOM_T	'v'
# define TOP_T		'w'
#endif
