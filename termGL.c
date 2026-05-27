#include "include/termGL.h"

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

# define TOP_ROW_COLOR_SEQ(rgb)		"38;2;" rgb
# define BOT_ROW_COLOR_SEQ(rgb)		"48;2;" rgb
# define COLOR_SEQ_END		"m"

# define TOP_ROW_COLOR(rgb)		CSI TOP_ROW_COLOR_SEQ(rgb) COLOR_SEQ_END
# define BOT_ROW_COLOR(rgb)		CSI BOT_ROW_COLOR_SEQ(rgb) COLOR_SEQ_END

# define TWO_ROW_COLOR(rgb1, rgb2)	CSI TOP_ROW_COLOR_SEQ(rgb1) ";" BOT_ROW_COLOR_SEQ(rgb2) COLOR_SEQ_END
# define TWO_ROW_COLOR_SEQ_MAX_SIZE	(sizeof(TWO_ROW_COLOR("rrr;ggg;bbb", "rrr;ggg;bbb")) - 1)

# define ONE_ROW_COLOR(rgb)		TOP_ROW_COLOR(rgb)
# define ONE_ROW_COLOR_SEQ_MAX_SIZE	(sizeof(TOP_ROW_COLOR("rrr;ggg;bbb")) - 1)

# define PIXEL_TO_RGB(pix)		(pix & RED) >> 16, (pix & GREEN) >> 8, pix & BLUE

# define PIXEL_CHAR_OFFSET	3 * 8
# define PIXEL_CHAR_MARKER	(128 << PIXEL_CHAR_OFFSET)

#define INPUT_QUEUE_SIZE	20

#define ZBUF_NOTINIT	0
#define ZBUF_TEXT_LAYER	1

#define ABS(val)	(val < 0 ? (val) * -1 : val)
#define MAX(a, b)	(a > b ? a : b)

/* Special kind of Image containing the pixels to be displayed on screen.
  Can be used as an Image using the DISPLAY macro
  Upon calling renderDisplay(), the underlying image gets translated and put into a buffer of char before being sent to stdout. */
struct s_termgl {
	Image	content;
	char	* const buffer;
	int	framerate;
	useconds_t	timestep;
	useconds_t	last_frame_t;
	void	(*input_handler)(char, void *);
	void	*handler_context;
	struct termios	starting_term_state;
};

TermGL	termGLInit(const unsigned int width, const unsigned int height)
{
	const size_t	full_line_size = (TWO_ROW_COLOR_SEQ_MAX_SIZE + PIXEL_SIZE) * width + 1;
	const size_t	half_line_size = (ONE_ROW_COLOR_SEQ_MAX_SIZE + PIXEL_SIZE) * width + 1 + (sizeof(RESET_COLOR) - 1);

	const size_t	buffer_size = full_line_size * (height / 2) + half_line_size * (height % 2) + OVERHEAD_SIZE;
	char	*buffer = malloc(buffer_size);
	TermGL	ret = malloc(sizeof(struct s_termgl));

	strcpy(buffer, OVERHEAD_START);

	write(1, INIT_DISPLAY_SEQ, INIT_DISPLAY_SEQ_SIZE);

	struct s_termgl	initializer = {
		.content = initImage(width, height),
		.buffer = buffer + OVERHEAD_START_SIZE,
	};
	return (memcpy(ret, &initializer, sizeof(struct s_termgl)));
}

void	termGLDestroy(TermGL termGL)
{
	destroyImage(&termGL->content);
	free(termGL->buffer - OVERHEAD_START_SIZE);
	if (termGL->input_handler != NULL)
		restoreTerminalState(termGL);
	free(termGL);
}

unsigned int	getDisplayWidth(TermGL termGL)
{
	return (termGL->content.size[0]);
}

unsigned int	getDisplayHeight(TermGL termGL)
{
	return (termGL->content.size[1]);
}

void	setFramerate(const unsigned int frame_per_sec, TermGL termGL)
{
	termGL->framerate = frame_per_sec;
	termGL->timestep = 1 * 1000000 / frame_per_sec;
}

unsigned int	getFramerate(TermGL termGL)
{
	return (termGL->framerate);
}

void	registerInputHandler(void (*handler)(char, void *), void *handler_context, TermGL termGL)
{
	termGL->input_handler = handler;
	termGL->handler_context = handler_context;

	//setting terminal state
	tcgetattr(STDIN_FILENO, &termGL->starting_term_state);
	struct termios tcattr = termGL->starting_term_state;
	tcattr.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &tcattr);

	fcntl(0, F_SETFL, O_NONBLOCK);
}

void	restoreTerminalState(TermGL termGL)
{
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &termGL->starting_term_state);
}

static void	processInputs(TermGL termGL)
{
	char	buffer[INPUT_QUEUE_SIZE] = {0};
	int	read_amount;

	do {
		read_amount = read(0, buffer, INPUT_QUEUE_SIZE);
		for (int i = 0; i < read_amount; ++i)
			termGL->input_handler(buffer[i], termGL->handler_context);
	} while (read_amount == INPUT_QUEUE_SIZE);
}

static size_t	fillDisplayBuffer(TermGL termGL)
{
	size_t i = 0;
	Pixel_t	prev_top_pixel = UNDEFINED_PIXEL;
	Pixel_t	prev_bot_pixel = UNDEFINED_PIXEL;

	for (unsigned int y = 0; y < termGL->content.size[1] - 1; y += 2)
	{
		for (unsigned int x = 0; x < termGL->content.size[0]; ++x)
		{
			const Pixel_t	top_pixel = getPixel(x, y, (Image *)termGL);
			const Pixel_t	bot_pixel = getPixel(x, y + 1, (Image *)termGL);

			const int	write_bot_pixel = (bot_pixel != prev_bot_pixel);
			const int	write_top_pixel = ((top_pixel & 0xFFFFFF) != prev_top_pixel && top_pixel != bot_pixel);

			if (write_bot_pixel && write_top_pixel)
			{
				i += sprintf(&termGL->buffer[i], TWO_ROW_COLOR("%d;%d;%d", "%d;%d;%d"), PIXEL_TO_RGB(top_pixel), PIXEL_TO_RGB(bot_pixel));
				prev_top_pixel = top_pixel;
				prev_bot_pixel = bot_pixel;
			}
			else if (write_top_pixel)
			{
				i += sprintf(&termGL->buffer[i], TOP_ROW_COLOR("%d;%d;%d"), PIXEL_TO_RGB(top_pixel));
				prev_top_pixel = top_pixel;
			}
			else if (write_bot_pixel)
			{
				i += sprintf(&termGL->buffer[i], BOT_ROW_COLOR("%d;%d;%d"), PIXEL_TO_RGB(bot_pixel));
				prev_bot_pixel = bot_pixel;
			}

			if (top_pixel & PIXEL_CHAR_MARKER)
			{
				i += sprintf(&termGL->buffer[i], "%c", (top_pixel & (127 << PIXEL_CHAR_OFFSET)) >> PIXEL_CHAR_OFFSET);
				prev_top_pixel &= 0xFFFFFF;//remove PIXEL_CHAR_MARKER if present
			}
			else
				i += sprintf(&termGL->buffer[i], (top_pixel == bot_pixel ? " " : PIXEL_STR));
		}
		termGL->buffer[i++] = '\n';
	}
	if (termGL->content.size[1] & 1)
	{
		const unsigned int	y = termGL->content.size[1] - 1;

		i += sprintf(&termGL->buffer[i], RESET_COLOR);
		prev_top_pixel = UNDEFINED_PIXEL;
		for (unsigned int x = 0; x < termGL->content.size[0]; ++x)
		{
			const Pixel_t	top_pixel = getPixel(x, y, (Image *)termGL);

			if (top_pixel == prev_top_pixel)
				i += sprintf(&termGL->buffer[i], PIXEL_STR);
			else
			{
				i += sprintf(&termGL->buffer[i], TOP_ROW_COLOR("%d;%d;%d") PIXEL_STR,
						PIXEL_TO_RGB(top_pixel));
				prev_top_pixel = top_pixel;
			}
		}
		termGL->buffer[i++] = '\n';
	}
	return (strcpy(&termGL->buffer[i], OVERHEAD_END), i + OVERHEAD_END_SIZE);
}

static useconds_t	getCurrentTime(void)
{
	struct timeval	current_time;
	gettimeofday(&current_time, NULL);
	return (current_time.tv_usec);
}

void	renderDisplay(TermGL termGL)
{
	const size_t char_count = fillDisplayBuffer(termGL);
	clearImage((Image *)termGL);

	if (termGL->timestep != 0)
	{
		const useconds_t	elapsed_time = getCurrentTime() - termGL->last_frame_t;

		if (termGL->timestep > elapsed_time)
			usleep(termGL->timestep - elapsed_time);
	}

	write(1, termGL->buffer - OVERHEAD_START_SIZE, char_count + OVERHEAD_START_SIZE);

	if (termGL->timestep != 0)
		termGL->last_frame_t = getCurrentTime();

	if (termGL->input_handler != NULL)
		processInputs(termGL);
}

Image	initImage(const unsigned int width, const unsigned int height)
{
	return ((Image){
		.pixels = calloc(width * height, sizeof(Pixel_t)),
		.zbuffer = calloc(width * height, sizeof(unsigned int)),
		.size = {width, height},
	});
}

void	destroyImage(Image *image)
{
	free(image->pixels);
	free(image->zbuffer);
}

Pixel_t	getPixel(const unsigned int x, const unsigned int y, Image *img)
{
	return (img->pixels[x + y * img->size[0]]);
}

void	setPixel(const unsigned int x, const unsigned int y, Pixel_t value, Image *img)
{
	img->pixels[x + y * img->size[0]] = value;
}

/* only place a pixel if z is lower than the zbuffer value for this pixel */
void	setPixelZBuffered(const unsigned int x, const unsigned int y, const unsigned int z, Pixel_t value, Image *img)
{
	if (img->zbuffer[x + y * img->size[0]] != ZBUF_NOTINIT && z > img->zbuffer[x + y * img->size[0]])
		return ;
	img->pixels[x + y * img->size[0]] = value;
	img->zbuffer[x + y * img->size[0]] = z;
}

void	clearImage(Image *img)
{
	memset(img->pixels, 0, img->size[0] * img->size[1] * sizeof(Pixel_t));
	memset(img->zbuffer, 0, img->size[0] * img->size[1] * sizeof(unsigned int));
}

void	imageToImage(const Image *img, Image *dest, const unsigned int x, const unsigned int y)
{
	for (unsigned int j = 0; j < img->size[1]; ++j)
		memcpy(&dest->pixels[(y + j) * dest->size[0] + x], &img->pixels[img->size[0] * j], img->size[0] * sizeof(Pixel_t));
}

Image	strToImage(const char *str, const unsigned int width, const unsigned int height, const Pixel_t color)
{
	Pixel_t	*pixels = calloc(width * height, sizeof(Pixel_t));

	for (int i = 0; str[i]; ++i)
		if (str[i] != ' ' && str[i] != '0')
			pixels[i] = color;

	return ((Image){
		.pixels = pixels,
		.size = {width, height},
	});
}

float	degToRad(const float deg) { return (deg * M_PI / 180); }

/* unrolled matrix multiplication between point p and x rotation matrix */
fVec3	rotateX(fVec3 p, float angle_deg)
{
	if ((int)angle_deg == 0)
		return (p);
	const float	angle = degToRad(angle_deg);
	const float	cos = cosf(angle);
	const float	sin = sinf(angle);

	return ((fVec3){
		.x = p.x,
		.y = cos * p.y - sin * p.z,
		.z = sin * p.y + cos * p.z
	});
}

/* unrolled matrix multiplication between point p and y rotation matrix */
fVec3	rotateY(fVec3 p, float angle_deg)
{
	if ((int)angle_deg == 0)
		return (p);
	const float	angle = degToRad(angle_deg);
	const float	cos = cosf(angle);
	const float	sin = sinf(angle);

	return ((fVec3){
		.x = cos * p.x + sin * p.z,
		.y = p.y,
		.z = -sin * p.x + cos * p.z
	});
}

/* unrolled matrix multiplication between point p and z rotation matrix */
fVec3	rotateZ(fVec3 p, float angle_deg)
{
	if ((int)angle_deg == 0)
		return (p);
	const float	angle = degToRad(angle_deg);
	const float	cos = cosf(angle);
	const float	sin = sinf(angle);

	return ((fVec3){
		.x = cos * p.x - sin * p.y,
		.y = sin * p.x + cos * p.y,
		.z = p.z
	});
}

uintVec3	toAbsolute(fVec3 p, Image *img)
{
	return ((uintVec3){
		.x = (p.x + 1) * (img->size[0] / 2),
		.y = (p.y + 1) * (img->size[1] / 2),
		.z = (p.z + 1) * (ZBUF_AMPLITUDE / 2) + ZBUF_MIN_VALUE,
	});
}

/* 3D Bresenham's line algorithm */
void	drawLine(uintVec3 p0, uintVec3 p1, const Pixel_t color, Image *dest)
{
	const int	dx = ABS((int)(p1.x - p0.x));
	const int	dy = ABS((int)(p1.y - p0.y));
	const int	dz = ABS((int)(p1.z - p0.z));
	const int	dmax = MAX(dx, dy);

	const int	step_x = (p1.x > p0.x ? 1 : -1);
	const int	step_y = (p1.y > p0.y ? 1 : -1);
	const int	step_z = (p1.z > p0.z ? 1 : -1);

	int	err_x = dmax / 2;
	int	err_y = err_x;
	int	err_z = err_x;

	for(int i = dmax; i >= 0; --i)
	{
		setPixelZBuffered(p0.x, p0.y, p0.z, color, dest);
		err_x -= dx; if (err_x < 0) { err_x += dmax; p0.x += step_x; }
		err_y -= dy; if (err_y < 0) { err_y += dmax; p0.y += step_y; }
		err_z -= dz; if (err_z < 0) { err_z += dmax; p0.z += step_z; }
	}
}

void	drawPath(const Pixel_t color, Image *img, uintVec3 p0, ...)
{
	va_list	ap;
	va_start(ap, p0);

	uintVec3	point = p0;
	uintVec3	next_point;

	do {
		next_point = va_arg(ap, uintVec3);
		drawLine(point, next_point, color, img);
		point = next_point;
	} while (memcmp(&point, &p0, sizeof(uintVec3)));

	va_end(ap);
}

void	putText(const char *str, unsigned int x, unsigned int y, const Pixel_t font_color, const Pixel_t bg_color, Image *img)
{
	if (y & 1)
		y -= 1;
	for (int i = 0; str[i]; ++i)
	{
		setPixelZBuffered(x, y, ZBUF_TEXT_LAYER, font_color | (str[i] << PIXEL_CHAR_OFFSET) | PIXEL_CHAR_MARKER, img);
		setPixelZBuffered(x, y + 1, ZBUF_TEXT_LAYER, bg_color, img);
		if (++x > img->size[0])
		{
			x = 0;
			y += 2;
		}
	}
}
