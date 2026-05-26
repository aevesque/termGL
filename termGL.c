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

#define ABS(val)	(val < 0 ? (val) * -1 : val)

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
		.content = (Image) {
			.pixels = calloc(width * height, sizeof(Pixel_t)),
			.size = {width, height},
		},
		.buffer = buffer + OVERHEAD_START_SIZE,
	};
	return (memcpy(ret, &initializer, sizeof(struct s_termgl)));
}

void	termGLDestroy(TermGL termGL)
{
	free(termGL->content.pixels);
	free(termGL->buffer - OVERHEAD_START_SIZE);
	if (termGL->input_handler != NULL)
		restoreTerminalState(termGL);
	free(termGL);
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
		.size = {width, height},
	});
}

void	destroyImage(Image *image)
{
	free(image->pixels);
}

Pixel_t	getPixel(const unsigned int x, const unsigned int y, Image *img)
{
	return (img->pixels[x + y * img->size[0]]);
}

void	setPixel(const unsigned int x, const unsigned int y, Pixel_t value, Image *img)
{
	img->pixels[x + y * img->size[0]] = value;
}

void	clearImage(Image *img)
{
	memset(img->pixels, 0, img->size[0] * img->size[1] * sizeof(Pixel_t));
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
	});
}

/* Bresenham's line algorithm */
void	drawLine(uintVec3 p0, uintVec3 p1, const Pixel_t color, Image *dest)
{
	const int	dx = ABS((int)(p1.x - p0.x));
	const int	dy = -ABS((int)(p1.y - p0.y));
	const int	step_x = (p1.x > p0.x ? 1 : -1);
	const int	step_y = (p1.y > p0.y ? 1 : -1);
	int	err = dy + dx;
	int	err2;

	while (p0.x != p1.x || p0.y != p1.y)
	{
		setPixel(p0.x, p0.y, color, dest);
		err2 = 2 * err;
		if (err2 >= dy)
		{
			err += dy;
			p0.x += step_x;
		}
		if (err2 <= dx)
		{
			err += dx;
			p0.y += step_y;
		}
	}
	setPixel(p0.x, p0.y, color, dest);
}

/* ! this should not be called directly ! use the drawFace macro with the same arguments */
void	drawFace_internal(const Pixel_t color, Image *img, uintVec3 p0, ...)
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
		setPixel(x, y, font_color | (str[i] << PIXEL_CHAR_OFFSET) | PIXEL_CHAR_MARKER, img);
		setPixel(x, y + 1, bg_color, img);
		if (++x > img->size[0])
		{
			x = 0;
			y += 2;
		}
	}
}
