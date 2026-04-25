#include "include/termGL.h"

void	initDisplay(void)
{
	write(1, INIT_DISPLAY_SEQ, INIT_DISPLAY_SEQ_SIZE);
}

Frame	createFrame(const size_t width, const size_t height)
{
	const size_t	full_line_size = (FULL_ROW_COLOR_SEQ_MAX_SIZE + PIXEL_SIZE) * width + 1;
	const size_t	half_line_size = (HALF_ROW_COLOR_SEQ_MAX_SIZE + PIXEL_SIZE) * width + 1;

	const size_t	buffer_size = full_line_size * (height / 2) + half_line_size * (height % 2) + OVERHEAD_SIZE;
	char	*buffer = malloc(buffer_size);

	strcpy(buffer, OVERHEAD_START);
	strncpy(buffer + buffer_size - OVERHEAD_END_SIZE, OVERHEAD_END, OVERHEAD_END_SIZE);

	return ((Frame){
			.pixels = calloc(width * height, sizeof(int)),
			.size = {width, height},
			.buffer = buffer + OVERHEAD_START_SIZE,
			.buffer_size = buffer_size,
			});
}

void	destroyFrame(Frame *frame)
{
	free(frame->pixels);
	free(frame->buffer - OVERHEAD_START_SIZE);
}

void	clearFrame(Frame *frame)
{
	memset(frame->pixels, 0, frame->size[0] * frame->size[1] * sizeof(int));
}

static void	fillFrameBuffer(Frame *frame)
{
	size_t i = -1;

	for (size_t y = 0; y < frame->size[1] - 1; y += 2)
	{
		for (size_t x = 0; x < frame->size[0]; ++x)
		{
			const int	top_pixel = GET_PIXEL(x, y, frame);
			const int	bot_pixel = GET_PIXEL(x, y + 1, frame);

			i += sprintf(&frame->buffer[++i], FULL_ROW_COLOR_SEQ("%d;%d;%d", "%d;%d;%d") PIXEL_STR,
					RGB_R_VALUE(top_pixel), RGB_G_VALUE(top_pixel), RGB_B_VALUE(top_pixel),
					RGB_R_VALUE(bot_pixel), RGB_G_VALUE(bot_pixel), RGB_B_VALUE(bot_pixel)) - 1; //-1 since sprintf appends NULL
		}
		frame->buffer[++i] = '\n';
	}
	if (frame->size[1] % 2)
	{
		const size_t	y = frame->size[1] - 1;

		for (size_t x = 0; x < frame->size[0]; ++x)
		{
			const int	top_pixel = GET_PIXEL(x, y, frame);

			i += sprintf(&frame->buffer[++i], HALF_ROW_COLOR_SEQ("%d;%d;%d") PIXEL_STR,
					RGB_R_VALUE(top_pixel), RGB_G_VALUE(top_pixel), RGB_B_VALUE(top_pixel)) - 1;
		}
		frame->buffer[++i] = '\n';
	}
}

void	displayFrame(Frame *frame)
{
	memset(frame->buffer, 0, frame->buffer_size - OVERHEAD_SIZE);
	fillFrameBuffer(frame);
	write(1, frame->buffer - OVERHEAD_START_SIZE, frame->buffer_size);
}

Image	createImage(const size_t width, const size_t height, const int *src)
{
	int	*pixels = calloc(width * height, sizeof(int));

	if (src)
		memcpy(pixels, src, width * height * sizeof(int));

	return ((Image){
			.pixels = pixels,
			.size = {width, height},
			});
}

void	destroyImage(Image *image)
{
	free(image->pixels);
}

void	putImageInFrame(const Image *image, Frame *target, const size_t x, const size_t y)
{
	for (size_t j = 0; j < image->size[1]; ++j)
		memcpy(&target->pixels[(y + j) * target->size[0] + x], &image->pixels[image->size[0] * j], image->size[0] * sizeof(int));
}

Image	strToNewImage(const char *str, const size_t width, const size_t height, const int color)
{
	int	*pixels = calloc(width * height, sizeof(int));
	int	i = -1;

	while (str[++i])
		if (str[i] != ' ' && str[i] != '0')
			pixels[i] = color;

	return ((Image){
			.pixels = pixels,
			.size = {width, height},
			});
}
