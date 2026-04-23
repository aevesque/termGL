#include "include/termGL.h"

void	initDisplay(void)
{
	write(1, SCROLL_UP, sizeof(SCROLL_UP) - 1);
}

Frame	createFrame(const size_t size[2])
{
#if FORCE_MONOSPACE
	const size_t	display_buffer_size = (size[0] * 2 + 1) * size[1] + DISPLAY_OVERHEAD_SIZE;
#else
	const size_t	display_buffer_size = (size[0] + 1) * size[1] + DISPLAY_OVERHEAD_SIZE;
#endif
	char *display_buffer = malloc(display_buffer_size);

	strcpy(display_buffer, DISPLAY_OVERHEAD_START);
	strcpy(display_buffer + display_buffer_size - DISPLAY_OVERHEAD_END_SIZE, DISPLAY_OVERHEAD_END);

	return ((Frame){
			.pixels = malloc(size[0] * size[1]),
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
	memset(frame->pixels, 0, frame->size[0] * frame->size[1]);
}

static void	fillFrameDisplayBuffer(Frame *frame)
{
	unsigned int buffer_i = -1;

	for (unsigned int y = 0; y < frame->size[1]; ++y)
	{
		for (unsigned int x = 0; x < frame->size[0]; ++x)
		{
			const char value = frame->pixels[x + y * frame->size[0]];

			frame->display_buffer[++buffer_i] = value;
#if FORCE_MONOSPACE
			frame->display_buffer[++buffer_i] = value;
#endif
		}
		frame->display_buffer[++buffer_i] = '\n';
	}
}

void	displayFrame(Frame *frame)
{
	fillFrameDisplayBuffer(frame);
	write(1, frame->display_buffer - DISPLAY_OVERHEAD_START_SIZE, frame->display_buffer_size);
	memset(frame->display_buffer, 0, frame->display_buffer_size - DISPLAY_OVERHEAD_SIZE);
}

Image	createImage(const size_t size[2], const char *str)
{
	if (!str)
		return ((Image){ .size = {size[0], size[1]}});

	return ((Image){
			.content = strncpy(malloc(size[0] * size[1]), str, size[0] * size[1]),
			.size = {size[0], size[1]},
			});
}

void	destroyImage(Image *image)
{
	free(image->content);
}

void	putImageToFrame(const Image *image, Frame *frame, const size_t pos[2])
{
	for (unsigned int y = 0; y < image->size[1]; ++y)
		memcpy(&frame->pixels[(pos[1] + y) * frame->size[0] + pos[0]], &image->content[image->size[0] * y], image->size[0]);
}
