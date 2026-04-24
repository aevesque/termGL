#include "include/termGL.h"

void	initDisplay(void)
{
	write(1, SCROLL_UP, sizeof(SCROLL_UP) - 1);
}

Frame	createFrame(const size_t size[2])
{
	const size_t	double_line_size = (DOUBLE_COLOR_SEQ_MAX_SIZE + sizeof(PIXEL_STR) - 1) * size[0] + 1;
	const size_t	single_line_size = (SINGLE_COLOR_SEQ_MAX_SIZE + sizeof(PIXEL_STR) - 1) * size[0] + 1;

	const size_t	display_buffer_size = double_line_size * (size[1] / 2) + single_line_size * (size[1] % 2) + DISPLAY_OVERHEAD_SIZE;
	char *display_buffer = malloc(display_buffer_size);

	strcpy(display_buffer, DISPLAY_OVERHEAD_START);
	strncpy(display_buffer + display_buffer_size - DISPLAY_OVERHEAD_END_SIZE, DISPLAY_OVERHEAD_END, DISPLAY_OVERHEAD_END_SIZE);

	return ((Frame){
			.pixels = malloc(size[0] * size[1] * sizeof(int)),
			.size = {size[0], size[1]},
			.display_buffer = display_buffer + DISPLAY_OVERHEAD_START_SIZE,
			.display_buffer_size = display_buffer_size,
			});
}

void	destroyFrame(Frame *frame)
{
	free(frame->pixels);
	free(frame->display_buffer - DISPLAY_OVERHEAD_START_SIZE);
}

void	clearFrame(Frame *frame)
{
	memset(frame->pixels, 0, frame->size[0] * frame->size[1] * sizeof(int));
}

static void	fillFrameDisplayBuffer(Frame *frame)
{
	size_t i = -1;

	for (size_t y = 0; y < frame->size[1] - 1; y += 2)
	{
		for (size_t x = 0; x < frame->size[0]; ++x)
		{
			const int	top_pixel = GET_PIXEL(x, y, frame);
			const int	bot_pixel = GET_PIXEL(x, y + 1, frame);

			i += sprintf(&frame->display_buffer[++i], DOUBLE_COLOR_SEQ("%d;%d;%d", "%d;%d;%d") PIXEL_STR,
					RGB_R_VALUE(top_pixel), RGB_G_VALUE(top_pixel), RGB_B_VALUE(top_pixel),
					RGB_R_VALUE(bot_pixel), RGB_G_VALUE(bot_pixel), RGB_B_VALUE(bot_pixel)) - 1; //-1 since sprintf appends NULL
		}
		frame->display_buffer[++i] = '\n';
	}
	if (frame->size[1] % 2)
	{
		const size_t	y = frame->size[1] - 1;

		for (size_t x = 0; x < frame->size[0]; ++x)
		{
			const int	top_pixel = GET_PIXEL(x, y, frame);

			i += sprintf(&frame->display_buffer[++i], SINGLE_COLOR_SEQ("%d;%d;%d") PIXEL_STR,
					RGB_R_VALUE(top_pixel), RGB_G_VALUE(top_pixel), RGB_B_VALUE(top_pixel)) - 1;
		}
		frame->display_buffer[++i] = '\n';
	}
}

void	displayFrame(Frame *frame)
{
	fillFrameDisplayBuffer(frame);
	write(1, frame->display_buffer - DISPLAY_OVERHEAD_START_SIZE, frame->display_buffer_size);
	memset(frame->display_buffer, 0, frame->display_buffer_size - DISPLAY_OVERHEAD_SIZE);
}

Image	createImage(const size_t size[2], const int *src)
{
	if (!src)
		return ((Image){
				.pixels = malloc(size[0] * size[1] * sizeof(int)),
				.size = {size[0], size[1]},
				});

	return ((Image){
			.pixels = memcpy(malloc(size[0] * size[1] * sizeof(int)), src, size[0] * size[1] * sizeof(int)),
			.size = {size[0], size[1]},
			});
}

void	destroyImage(Image *image)
{
	free(image->pixels);
}

void	putImageToFrame(const Image *image, Frame *frame, const size_t pos[2])
{
	for (size_t y = 0; y < image->size[1]; ++y)
		memcpy(&frame->pixels[(pos[1] + y) * frame->size[0] + pos[0]], &image->pixels[image->size[0] * y], image->size[0] * sizeof(int));
}
