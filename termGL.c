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

/*  Characters are twice as tall as they are wide, even in graphic mode. To mitigate this, display_buffer is stretched by 2 in x */
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
