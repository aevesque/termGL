#include "termGL.h"

int main(void)
{
	Image	image = initImage((size_t []){2, 2});

	SAVE_CURSOR;

	image.content[0] = 1;
	image.content[3] = 1;

	displayImage(&image);
	usleep(DELAY_USEC);
	clearImage(&image);

	image.content[1] = 1;
	image.content[2] = 1;

	displayImage(&image);
	usleep(DELAY_USEC);
	clearImage(&image);

	image.content[0] = 1;
	image.content[3] = 1;

	displayImage(&image);

	destroyImage(&image);
	return (0);
}
