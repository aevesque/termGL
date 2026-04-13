#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define ESC	"\33"

#define SAVE_CURSOR_ORIGIN	write(1, ESC "7", 2)
#define CURSOR_TO_ORIGIN	write(1, ESC "8", 2)
#define CLEAR_SCREEN		write(1, ESC "[J", 3)

#define SET_GRAPHIC_MAPPING	write(1, ESC "(0", 3)
#define RESET_MAPPING		write(1, ESC "(B", 3)

typedef struct {
	char	*pixels;
	char	*display_buffer;
	unsigned int	size[2];
}		Image;

void	initDisplay(void);
void	destroyDisplay(void);
Image	createImage(unsigned int size[2]);
void	destroyImage(Image *image);
void	clearImage(Image *image);
void	fillImageDisplayBuffer(Image *image);
void	displayImage(Image *image);
