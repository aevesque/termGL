#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#ifndef TERM_GL
# define TERM_GL

/* CSI is used to start some ANSI console codes ; see man 4 console_codes */
# define CSI	"\033["

# define SCROLL_DOWN		CSI "2J"
# define CURSOR_TO_ORIGIN	CSI "H"
# define ERASE_DISPLAY		CSI "J"
# define RESET_COLOR		CSI "0m"

# define INIT_DISPLAY_SEQ	SCROLL_DOWN
# define INIT_DISPLAY_SEQ_SIZE	(sizeof(INIT_DISPLAY_SEQ) - 1)

# define OVERHEAD_START		CURSOR_TO_ORIGIN ERASE_DISPLAY
# define OVERHEAD_START_SIZE	(sizeof(OVERHEAD_START) - 1)

# define OVERHEAD_END		RESET_COLOR
# define OVERHEAD_END_SIZE	(sizeof(OVERHEAD_END) - 1)

# define OVERHEAD_SIZE	(OVERHEAD_START_SIZE + OVERHEAD_END_SIZE)

# define PIXEL_STR	"▄"	//unicode lower half block, 0xE2 0x96 0x84
# define PIXEL_SIZE	(sizeof(PIXEL_STR) - 1)

# define TOP_ROW_COLOR		"48;2;"
# define BOT_ROW_COLOR		"38;2;"
# define COLOR_SEQ_END		"m"

# define FULL_ROW_COLOR_SEQ(rgb1, rgb2)	CSI TOP_ROW_COLOR rgb1 ";" BOT_ROW_COLOR rgb2 COLOR_SEQ_END
# define FULL_ROW_COLOR_SEQ_MAX_SIZE	(sizeof(FULL_ROW_COLOR_SEQ("rrr;ggg;bbb", "rrr;ggg;bbb")) - 1)

# define HALF_ROW_COLOR_SEQ(rgb)		FULL_ROW_COLOR_SEQ(rgb, "0;0;0")
# define HALF_ROW_COLOR_SEQ_MAX_SIZE	(sizeof(HALF_ROW_COLOR_SEQ("rrr;ggg;bbb")) - 1)

# define RGB_R_VALUE(n)		(((0xFF << 16) & n) >> 16)
# define RGB_G_VALUE(n)		(((0xFF << 8) & n) >> 8)
# define RGB_B_VALUE(n)		(0xFF & n)

# define RED		0xFF0000
# define GREEN		0x00FF00
# define BLUE		0x0000FF
# define WHITE		0xFFFFFF
# define BLACK		0x000000


void	initDisplay(void);

typedef struct {
	int	* const pixels;
	const size_t	size[2];
	char	* const buffer;
	const size_t	buffer_size;
}		Frame;

Frame	createFrame(const size_t width, const size_t height);
void	destroyFrame(Frame *frame);

void	displayFrame(Frame *frame);
void	clearFrame(Frame *frame);

typedef struct {
	int	* const	pixels;
	const size_t	size[2];
}		Image;

Image	createImage(const size_t width, const size_t height, const int *src);
void	destroyImage(Image *image);

void	putImageInFrame(const Image *image, Frame *target, const size_t x, const size_t y);

/* fills every non empty pixel with color. Considers ' '(32) and '0'(48) empty */
Image	strToNewImage(const char *str, const size_t width, const size_t height, const int color);

/* Works with both Frame and Image */
# define GET_PIXEL(x, y, src)	src->pixels[(x) + (y) * src->size[0]]
# define SET_PIXEL(value, x, y, dest)	GET_PIXEL(x, y, dest) = value

#endif
