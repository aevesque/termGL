#include "termGL.h"

void	initDisplay(void)
{
	SET_GRAPHIC_MAPPING;
	SAVE_CURSOR_ORIGIN;
}

void	destroyDisplay(void)
{
	RESET_MAPPING;
}

Image	createImage(unsigned int size[2])
{
	return ((Image){
			.pixels = malloc(size[0] * size[1]),
			.display_buffer = malloc((size[0] * 2 + 1) * size[1]),
			.size = {size[0], size[1]},
			});
}

void	destroyImage(Image *image)
{
	free(image->pixels);
	free(image->display_buffer);
}

void	clearImage(Image *image)
{
	memset(image->pixels, 0, image->size[0] * image->size[1]);
}

/*  Characters are twice as tall as they are wide, even in graphic mode. To mitigate this, display_buffer is stretched by 2 in x */
void	fillImageDisplayBuffer(Image *image)
{
	unsigned int buffer_i = -1;

	for (unsigned int y = 0; y < image->size[1]; ++y)
	{
		for (unsigned int x = 0; x < image->size[0]; ++x)
		{
			const char value = image->pixels[x + y * image->size[0]];

			image->display_buffer[++buffer_i] = value;
			image->display_buffer[++buffer_i] = value;
		}
		image->display_buffer[++buffer_i] = '\n';
	}
}

void	displayImage(Image *image)
{
	CURSOR_TO_ORIGIN;
	CLEAR_SCREEN;
	fillImageDisplayBuffer(image);
	write(1, image->display_buffer, (image->size[0] * 2 + 1) * image->size[1]);
	memset(image->display_buffer, 0, (image->size[0] * 2 + 1) * image->size[1]);
}
