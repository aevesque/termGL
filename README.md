
# termGL

A minimal graphics library in C for the Linux terminal.

Uses ANSI sequences and Unicode to display colored pixels.

## Requirements

- A terminal with ANSI colors and Unicode support

## Dependencies

- Make (optional, this is a single file library)

## Linking

Linking with the static library requires the -lm flag.

## Example

````
    gcc sample/cube.c termGL.a -lm
````

## API Reference

````
typedef struct s_termgl	*TermGL;

TermGL	termGLInit(const unsigned int width, const unsigned int height);
void	termGLDestroy(TermGL termGL);

unsigned int	getDisplayWidth(TermGL termGL);
unsigned int	getDisplayHeight(TermGL termGL);

/* setting a framerate allows termGL to automatically time frames */
void	setFramerate(const unsigned int frame_per_sec, TermGL termGL);
unsigned int	getFramerate(TermGL termGL);

/* display an fps counter on the top left of the current frame based on the time to render and print the last frame */
void	showFPS(TermGL termGL);

/* supported inputs are ascii characters ; escaped sequences (F1 - F12, arrow keys etc) are not supported.
 input handler gets called with the detected keycode as first argument and handler_context as it's second argument */
void	registerInputHandler(void (*handler)(char, void *), void *handler_context, TermGL termGL);

/* registering an input handler changes the terminal state beyond the program's scope. Calling termGLDestroy will revert
the terminal to it's previous state as part of cleanup.
 On crash, most modern terminal automatically revert themselves to a known safe state ; if this isn't the case this
fonction needs to be called */
void	restoreTerminalState(TermGL termgl);

void	renderDisplay(TermGL termGL);

typedef unsigned int Pixel_t;

/* Fixed-size 2d pixel buffer. */
typedef struct {
	Pixel_t	* const	pixels;
	unsigned int * const zbuffer;
	const unsigned int	size[2];
}		Image;

/* use the DISPLAY macro in functions requiring an Image * to directly write to the display instead */
#define DISPLAY(termGL)		((Image *)termGL)

Image	initImage(const unsigned int width, const unsigned int height);
void	destroyImage(Image *img);

Pixel_t	getPixel(const unsigned int x, const unsigned int y, Image *img);
void	setPixel(const unsigned int x, const unsigned int y, Pixel_t value, Image *img);

/* only place a pixel if z is lower than or equal to the zbuffer value for this pixel
 pixels with a zbuffer value of 0 are excluded from this and have the lowest priority */
void	setPixelZBuffered(const unsigned int x, const unsigned int y, const unsigned int z, Pixel_t value, Image *img);

void	clearImage(Image *img);

/* copy an image into another image ; can be used to display images by using the DISPLAY macro as dest */
void	imageToImage(const Image *img, Image *dest, const unsigned int x, const unsigned int y);

/* creates an image following the chars in the str.
Every char create a color pixel except ' '(32) and '0'(48) wich are black */
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

/* convert a fVec3 in relative coordinates (-1 - +1)
 to a uintVec3 in absolute coordinates (0 - img->width, 0 - img->height, ZBUF_MIN_VALUE - ZBUF_AMPLITUDE + ZBUF_MIN_VALUE) */
uintVec3	toAbsolute(fVec3 p, Image *img);

typedef struct {
	int	x;
	int	y;
	int	z;
}	iVec3;

/* convert a fVec3 in relative coordinates without boundaries to an absolute coordinate (using the image size as a reference) without boundaries (see toAbsolute) */
iVec3	toAbsoluteUnbound(fVec3 p, Image *img);

void	drawLine(uintVec3 p0, uintVec3 p1, const Pixel_t color, Image *dest);

/* draws a line between each point and the next */
void	drawPath(const Pixel_t color, Image *img, uintVec3 p0, ...);

/* draws a line between each point and the next, wrapping back to p0 */
void	drawClosedPath(const Pixel_t color, Image *img, uintVec3 p0, ...);

/* render an str instead of pixels.
 chars are 2 lines tall and are rendered on even lines only (line - 1 if line is odd) with a special zbuffer value as to always be on top */
void	putText(const char *str, unsigned int x, unsigned int y, const Pixel_t font_color, const Pixel_t bg_color, Image *img);
````
