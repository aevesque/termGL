#include "include/termGL.h"

void	initDisplay(void)
{
	write(1, INIT_DISPLAY_SEQ, INIT_DISPLAY_SEQ_SIZE);
}

Window	initWindow(const unsigned int width, const unsigned int height)
{
	const size_t	full_line_size = (TWO_ROW_COLOR_SEQ_MAX_SIZE + PIXEL_SIZE) * width + 1;
	const size_t	half_line_size = (ONE_ROW_COLOR_SEQ_MAX_SIZE + PIXEL_SIZE) * width + 1 + (sizeof(RESET_COLOR) - 1);

	const size_t	buffer_size = full_line_size * (height / 2) + half_line_size * (height % 2) + OVERHEAD_SIZE;
	char	*buffer = malloc(buffer_size);

	strcpy(buffer, OVERHEAD_START);
	return ((Window) {
		.content = (Image) {
			.pixels = calloc(width * height, sizeof(Pixel_t)),
			.size = {width, height},
		},
		.buffer = buffer + OVERHEAD_START_SIZE,
	});
}

void	destroyWindow(Window *win)
{
	free(win->content.pixels);
	free(win->buffer - OVERHEAD_START_SIZE);
}

static size_t	fillWindowBuffer(Window *win)
{
	size_t i = 0;
	Pixel_t	prev_top_pixel = UNDEFINED_PIXEL;
	Pixel_t	prev_bot_pixel = UNDEFINED_PIXEL;

	for (unsigned int y = 0; y < win->content.size[1] - 1; y += 2)
	{
		for (unsigned int x = 0; x < win->content.size[0]; ++x)
		{
			const Pixel_t	top_pixel = getPixel(x, y, (Image *)win);
			const Pixel_t	bot_pixel = getPixel(x, y + 1, (Image *)win);

			const int	write_bot_pixel = (bot_pixel != prev_bot_pixel);
			const int	write_top_pixel = (top_pixel != prev_top_pixel && top_pixel != bot_pixel);

			if (write_bot_pixel && write_top_pixel)
			{
				i += sprintf(&win->buffer[i], TWO_ROW_COLOR("%d;%d;%d", "%d;%d;%d") PIXEL_STR,
						PIXEL_TO_RGB(top_pixel), PIXEL_TO_RGB(bot_pixel));
				prev_top_pixel = top_pixel;
				prev_bot_pixel = bot_pixel;
			}
			else if (write_top_pixel)
			{
				i += sprintf(&win->buffer[i], TOP_ROW_COLOR("%d;%d;%d") PIXEL_STR,
						PIXEL_TO_RGB(top_pixel));
				prev_top_pixel = top_pixel;
			}
			else if (write_bot_pixel)
			{
				i += sprintf(&win->buffer[i], BOT_ROW_COLOR("%d;%d;%d") "%s",
						PIXEL_TO_RGB(bot_pixel), (top_pixel == bot_pixel ? " " : PIXEL_STR));
				prev_bot_pixel = bot_pixel;
			}
			else
                               i += sprintf(&win->buffer[i], (top_pixel == bot_pixel ? " " : PIXEL_STR));

		}
		win->buffer[i++] = '\n';
	}
	if (win->content.size[1] & 1)
	{
		const unsigned int	y = win->content.size[1] - 1;

		i += sprintf(&win->buffer[i], RESET_COLOR);
		prev_top_pixel = UNDEFINED_PIXEL;
		for (unsigned int x = 0; x < win->content.size[0]; ++x)
		{
			const Pixel_t	top_pixel = getPixel(x, y, (Image *)win);

			if (top_pixel == prev_top_pixel)
				i += sprintf(&win->buffer[i], PIXEL_STR);
			else
			{
				i += sprintf(&win->buffer[i], TOP_ROW_COLOR("%d;%d;%d") PIXEL_STR,
						PIXEL_TO_RGB(top_pixel));
				prev_top_pixel = top_pixel;
			}
		}
		win->buffer[i++] = '\n';
	}
	return (strcpy(&win->buffer[i], OVERHEAD_END), i + OVERHEAD_END_SIZE);
}

void	renderWindow(Window *win)
{
	const size_t char_count = fillWindowBuffer(win);
	write(1, win->buffer - OVERHEAD_START_SIZE, char_count + OVERHEAD_START_SIZE);
	clearImage((Image *)win);
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

void	imageToWindow(const Image *img, Window *win, const unsigned int x, const unsigned int y)
{
	for (unsigned int j = 0; j < img->size[1]; ++j)
		memcpy(&win->content.pixels[(y + j) * win->content.size[0] + x], &img->pixels[img->size[0] * j], img->size[0] * sizeof(Pixel_t));
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

/* unrolled matrix multiplication between Point p and x rotation matrix */
Point	rotateX(Point p, float angle_deg)
{
	if ((int)angle_deg == 0)
		return (p);
	const float	angle = degToRad(angle_deg);
	const float	cos = cosf(angle);
	const float	sin = sinf(angle);

	return ((Point){
		.x = p.x,
		.y = cos * p.y - sin * p.z,
		.z = sin * p.y + cos * p.z
	});
}

/* unrolled matrix multiplication between Point p and y rotation matrix */
Point	rotateY(Point p, float angle_deg)
{
	if ((int)angle_deg == 0)
		return (p);
	const float	angle = degToRad(angle_deg);
	const float	cos = cosf(angle);
	const float	sin = sinf(angle);

	return ((Point){
		.x = cos * p.x + sin * p.z,
		.y = p.y,
		.z = -sin * p.x + cos * p.z
	});
}

/* unrolled matrix multiplication between Point p and z rotation matrix */
Point	rotateZ(Point p, float angle_deg)
{
	if ((int)angle_deg == 0)
		return (p);
	const float	angle = degToRad(angle_deg);
	const float	cos = cosf(angle);
	const float	sin = sinf(angle);

	return ((Point){
		.x = cos * p.x - sin * p.y,
		.y = sin * p.x + cos * p.y,
		.z = p.z
	});
}

Point2D	toAbsolute(Point p, Image *img)
{
	return ((Point2D){
		.x = (p.x + 1) * (img->size[0] / 2),
		.y = (p.y + 1) * (img->size[1] / 2)
	});
}

/* Bresenham's line algorithm */
void	drawLine(Point2D p0, Point2D p1, const Pixel_t color, Image *dest)
{
	const int	dx = TERMGL_ABS((int)(p1.x - p0.x));
	const int	dy = -TERMGL_ABS((int)(p1.y - p0.y));
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
