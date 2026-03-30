#include "termGL.h"

Image	initImage(size_t size[2])
{
	return ((Image){
			.content = malloc(size[0] * size[1]),
			.size = {size[0], size[1]},
			});
}

void	clearImage(Image *image)
{
	memset(image->content, 0, image->size[0] * image->size[1]);
}

void	destroyImage(Image *image)
{
	free(image->content);
}

void	displayImage(Image *image)
{
	RESTORE_CURSOR;
	CLEAR;
	for (size_t y = 0; y < image->size[1]; y++)
	{
		for (size_t x = 0; x < image->size[0]; x++)
		{
			if (image->content[x + y * image->size[0]])
				write(1, PIXEL, PIXEL_SIZE);
			else
				write(1, EMPTY_PIXEL, EMPTY_PIXEL_SIZE);
		}
		write(1, "\n", 1);
	}
}

