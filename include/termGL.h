#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdarg.h>
#include <sys/time.h>
#include <fcntl.h>
#include <termios.h>

#ifndef TERM_GL
# define TERM_GL

typedef struct s_termgl	*TermGL;

TermGL	termGLInit(const unsigned int width, const unsigned int height);
void	termGLDestroy(TermGL termGL);

unsigned int	getDisplayWidth(TermGL termGL);
unsigned int	getDisplayHeight(TermGL termGL);

void	setFramerate(const unsigned int frame_per_sec, TermGL termGL);
unsigned int	getFramerate(TermGL termGL);

/* supported inputs are ascii characters ; escaped sequences (F1 - F12, arrow keys etc) are not supported
  input handler will get called with the detected keypress as it's first argument and handler_context as it's second argument */
void	registerInputHandler(void (*handler)(char, void *), void *handler_context, TermGL termGL);
/* registering an input handler changes the terminal state beyond the program's scope. Calling termGLDestroy will revert the terminal to it's previous state
  On crash, most modern terminal automatically revert themselves to a known safe state ; if this isn't the case this fonction needs to be called */
void	restoreTerminalState(TermGL termgl);

void	renderDisplay(TermGL termGL);

# define PIXEL_STR	"▀"	//unicode upper half block, 0xE2 0x96 0x80
# define PIXEL_SIZE	(sizeof(PIXEL_STR) - 1)

typedef unsigned int Pixel_t;

# define RED		0xFF0000
# define GREEN		0x00FF00
# define BLUE		0x0000FF
# define WHITE		0xFFFFFF
# define BLACK		0x000000
# define UNDEFINED_PIXEL	-1

/* Fixed-size 2d pixel buffer. */
typedef struct {
	Pixel_t	* const	pixels;
	unsigned int * const zbuffer;
	const unsigned int	size[2];
}		Image;

/* use the DISPLAY macro in functions requiring an Image * to directly write to the display instead of an image */
#define DISPLAY(termGL)		((Image *)termGL)

Image	initImage(const unsigned int width, const unsigned int height);
void	destroyImage(Image *img);

#define ZBUF_MIN_VALUE	2
#define ZBUF_AMPLITUDE	1000

Pixel_t	getPixel(const unsigned int x, const unsigned int y, Image *img);
void	setPixel(const unsigned int x, const unsigned int y, Pixel_t value, Image *img);
/* only place a pixel if z is lower than the zbuffer value for this pixel */
void	setPixelZBuffered(const unsigned int x, const unsigned int y, const unsigned int z, Pixel_t value, Image *img);

void	clearImage(Image *img);
void	imageToImage(const Image *img, Image *dest, const unsigned int x, const unsigned int y);

/* fills every non empty pixel with color. Considers ' '(32) and '0'(48) empty -> black */
Image	strToImage(const char *str, const unsigned int width, const unsigned int height, const Pixel_t color);

typedef struct {
	float	x;
	float	y;
	float	z;
}	fVec3;

float	degToRad(const float deg);

/* rotates a point around the origin on the specified axis */
fVec3	rotateX(fVec3 p, float angle_deg);
fVec3	rotateY(fVec3 p, float angle_deg);
fVec3	rotateZ(fVec3 p, float angle_deg);

typedef struct {
	unsigned int	x;
	unsigned int	y;
	unsigned int	z;
}	uintVec3;

/* convert a fVec3 using relative coordinates (-1 - +1) to a uintVec3 in absolute coordinates (0 - img->width and 0 - img->height) */
uintVec3	toAbsolute(fVec3 p, Image *img);

void	drawLine(uintVec3 p0, uintVec3 p1, const Pixel_t color, Image *dest);

/* draws a line between each point and the next, wrapping back to p0 */
#define	drawFace(color, img, p0, ...)	drawFace_internal(color, img, p0, __VA_ARGS__, p0)
void	drawFace_internal(const Pixel_t color, Image *img, uintVec3 p0, ...); //this should not be called directly

/* draw characters instead of pixels
 Text wraps around lines
 Text is always 2 pixel tall and gets drawn on even y (y - 1 if y is odd)*/
void	putText(const char *str, unsigned int x, unsigned int y, const Pixel_t font_color, const Pixel_t bg_color, Image *img);
#endif
