#include "include/termGL.h"

void	initDisplay(void)
{
	write(1, INIT_DISPLAY_SEQ, INIT_DISPLAY_SEQ_SIZE);
}

Window	initWindow(const size_t width, const size_t height)
{
	const size_t	full_line_size = (FULL_ROW_COLOR_SEQ_MAX_SIZE + PIXEL_SIZE) * width + 1;
	const size_t	half_line_size = (HALF_ROW_COLOR_SEQ_MAX_SIZE + PIXEL_SIZE) * width + 1;

	const size_t	buffer_size = full_line_size * (height / 2) + half_line_size * (height % 2) + OVERHEAD_SIZE;
	char	*buffer = malloc(buffer_size);

	strcpy(buffer, OVERHEAD_START);
	strncpy(buffer + buffer_size - OVERHEAD_END_SIZE, OVERHEAD_END, OVERHEAD_END_SIZE);

	return ((Window) {
		.content = (Image) {
			.pixels = calloc(width * height, sizeof(Pixel_t)),
			.size = {width, height},
		},
		.buffer = buffer + OVERHEAD_START_SIZE,
		.buffer_size = buffer_size,
	});
}

void	destroyWindow(Window *win)
{
	free(win->content.pixels);
	free(win->buffer - OVERHEAD_START_SIZE);
}

static void	fillWindowBuffer(Window *win)
{
	size_t i = -1;

	for (size_t y = 0; y < win->content.size[1] - 1; y += 2)
	{
		for (size_t x = 0; x < win->content.size[0]; ++x)
		{
			const Pixel_t	top_pixel = getPixel(x, y, (Image *)win);
			const Pixel_t	bot_pixel = getPixel(x, y + 1, (Image *)win);

			i += sprintf(&win->buffer[++i], FULL_ROW_COLOR_SEQ("%d;%d;%d", "%d;%d;%d") PIXEL_STR,
					PIXEL_TO_RGB(top_pixel), PIXEL_TO_RGB(bot_pixel));
		}
		win->buffer[++i] = '\n';
	}
	if (win->content.size[1] % 2)
	{
		const size_t	y = win->content.size[1] - 1;

		for (size_t x = 0; x < win->content.size[0]; ++x)
		{
			const Pixel_t	top_pixel = getPixel(x, y, (Image *)win);

			i += sprintf(&win->buffer[++i], HALF_ROW_COLOR_SEQ("%d;%d;%d") PIXEL_STR,
					PIXEL_TO_RGB(top_pixel));
		}
		win->buffer[++i] = '\n';
	}
}

void	renderWindow(Window *win)
{
	fillWindowBuffer(win);
	write(1, win->buffer - OVERHEAD_START_SIZE, win->buffer_size);
	memset(win->buffer, 0, win->buffer_size - OVERHEAD_SIZE);
	clearImage((Image *)win);
}

Image	initImage(const size_t width, const size_t height)
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

Pixel_t	getPixel(const size_t x, const size_t y, Image *img)
{
	return (img->pixels[x + y * img->size[0]]);
}

void	setPixel(const size_t x, const size_t y, Pixel_t value, Image *img)
{
	img->pixels[x + y * img->size[0]] = value;
}

void	clearImage(Image *img)
{
	memset(img->pixels, 0, img->size[0] * img->size[1] * sizeof(Pixel_t));
}

void	imageToWindow(const Image *img, Window *win, const size_t x, const size_t y)
{
	for (size_t j = 0; j < img->size[1]; ++j)
		memcpy(&win->content.pixels[(y + j) * win->content.size[0] + x], &img->pixels[img->size[0] * j], img->size[0] * sizeof(Pixel_t));
}

Image	strToImage(const char *str, const size_t width, const size_t height, const Pixel_t color)
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
