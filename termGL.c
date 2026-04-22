#include "include/termGL.h"

void	initDisplay(void)
{
	SET_GRAPHIC_MAPPING;
	SAVE_CURSOR_ORIGIN;
}

void	destroyDisplay(void)
{
	RESET_GRAPHIC_MAPPING;
}

Frame	createFrame(unsigned int size[2])
{
	return ((Frame){
			.pixels = malloc(size[0] * size[1]),
			.display_buffer = malloc((size[0] * 2 + 1) * size[1]),
			.size = {size[0], size[1]},
			});
}

void	destroyFrame(Frame *frame)
{
	free(frame->pixels);
	free(frame->display_buffer);
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
	MOVE_CURSOR_TO_ORIGIN;
	CLEAR_SCREEN;
	fillFrameDisplayBuffer(frame);
	write(1, frame->display_buffer, (frame->size[0] * 2 + 1) * frame->size[1]);
	memset(frame->display_buffer, 0, (frame->size[0] * 2 + 1) * frame->size[1]);
}

Image	strToImage(unsigned int size[2], char *str)
{
	return ((Image){
			.content = strncpy(malloc(size[0] * size[1]), str, size[0] * size[1]),
			.size = {size[0], size[1]},
			});
}

void	destroyImage(Image *image)
{
	free(image->content);
}

void	putImageToFrame(Image *image, Frame *frame, unsigned int x, unsigned int y)
{
	if (x + image->size[0] > frame->size[0] || y + image->size[1] > frame->size[1])
		return ;
	for (unsigned int j = 0; j < image->size[1]; ++j)
		memcpy(&frame->pixels[(y + j) * frame->size[0] + x], &image->content[image->size[0] * j], image->size[0]);
}
