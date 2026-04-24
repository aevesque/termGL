#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#ifndef TERM_GL
# define TERM_GL

/* CSI is used to start some ANSI console codes ; see man 4 console_codes */
# define CSI	"\033["

# define SCROLL_UP		CSI "2J"

# define CURSOR_TO_ORIGIN	CSI "H"
# define ERASE_DISPLAY		CSI "J"
# define RESET_COLOR		CSI "0m"

# define DISPLAY_OVERHEAD_START		CURSOR_TO_ORIGIN ERASE_DISPLAY
# define DISPLAY_OVERHEAD_START_SIZE	(sizeof(DISPLAY_OVERHEAD_START) - 1)

# define DISPLAY_OVERHEAD_END		RESET_COLOR
# define DISPLAY_OVERHEAD_END_SIZE	(sizeof(DISPLAY_OVERHEAD_END) - 1)

# define DISPLAY_OVERHEAD_SIZE	(DISPLAY_OVERHEAD_START_SIZE + DISPLAY_OVERHEAD_END_SIZE)


# define PIXEL_STR	"▄"	//unicode lower half block, 0xE2 0x96 0x84

# define EVEN_ROW_COLOR		"48;2;"
# define ODD_ROW_COLOR		"38;2;"
# define COLOR_SEQ_END		"m"

# define DOUBLE_COLOR_SEQ(rgb1, rgb2)	CSI EVEN_ROW_COLOR rgb1 ";" ODD_ROW_COLOR rgb2 COLOR_SEQ_END
# define DOUBLE_COLOR_SEQ_MAX_SIZE	(sizeof(DOUBLE_COLOR_SEQ("rrr;ggg;bbb", "rrr;ggg;bbb")) - 1)

# define SINGLE_COLOR_SEQ(rgb)		DOUBLE_COLOR_SEQ(rgb, "0;0;0")
# define SINGLE_COLOR_SEQ_MAX_SIZE	(sizeof(SINGLE_COLOR_SEQ("rrr;ggg;bbb")) - 1)

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
	char	* const display_buffer;
	const size_t	display_buffer_size;
}		Frame;

Frame	createFrame(const size_t size[2]);
void	destroyFrame(Frame *frame);

void	displayFrame(Frame *frame);
void	clearFrame(Frame *frame);

typedef struct {
	int	* const pixels;
	const size_t size[2];
}		Image;

Image	createImage(const size_t size[2], const int *src);
void	destroyImage(Image *image);

void	putImageToFrame(const Image *image, Frame *frame, const size_t pos[2]);

# define GET_PIXEL(x, y, src)	src->pixels[(x) + (y) * src->size[0]]
# define SET_PIXEL(value, x, y, dest)	GET_PIXEL(x, y, dest) = value

#endif
