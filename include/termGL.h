#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#ifndef TERM_GL
# define TERM_GL

/* CSI is used to start some ANSI console codes ; see console_codes(4) */
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

# define Pixel_t	int

# define RED		0xFF0000
# define GREEN		0x00FF00
# define BLUE		0x0000FF
# define WHITE		0xFFFFFF
# define BLACK		0x000000

# define PIXEL_TO_RGB(pix)		(pix & RED) >> 16, (pix & GREEN) >> 8, pix & BLUE

/* Fixed-size 2d pixel buffer. */
typedef struct {
	Pixel_t	* const	pixels;
	const size_t	size[2];
}		Image;

/* Special kind of Image containing the pixels to be displayed on screen.
  Castable to Image.
  Upon calling renderWindow(), the window's content get translated and put in buffer before being sent to stdout.
  */
typedef struct {
	Image	content;
	char	* const buffer;
	const size_t	buffer_size;
}		Window;

void	initDisplay(void);

Window	initWindow(const size_t width, const size_t height);
void	destroyWindow(Window *win);

void	renderWindow(Window *win);

Image	initImage(const size_t width, const size_t height);
void	destroyImage(Image *img);

Pixel_t	getPixel(const size_t x, const size_t y, Image *img);
void	setPixel(const size_t x, const size_t y, Pixel_t value, Image *img);

void	clearImage(Image *img);
void	imageToWindow(const Image *img, Window *win, const size_t x, const size_t y);

/* fills every non empty pixel with color. Considers ' '(32) and '0'(48) empty -> black */
Image	strToImage(const char *str, const size_t width, const size_t height, const Pixel_t color);

#endif
