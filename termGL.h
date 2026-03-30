#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// pixels are 2 long since characters are twice as tall as they are wide
#define PIXEL_CHAR	"\342\226\210"
#define PIXEL		PIXEL_CHAR PIXEL_CHAR
#define PIXEL_SIZE	6

#define EMPTY_PIXEL	"  "
#define EMPTY_PIXEL_SIZE	2

#define ESC	"\33"

#define SAVE_CURSOR	write(1, ESC "7", 2);
#define RESTORE_CURSOR	write(1, ESC "8", 2);
#define CLEAR		write(1, ESC "[J", 3);

#define DELAY_MS	500
#define DELAY_USEC	DELAY_MS * 1000

typedef struct {
	char	*content;
	size_t	size[2];
}		Image;

Image	initImage(size_t size[2]);
void	displayImage(Image *image);
void	clearImage(Image *image);
void	destroyImage(Image *image);
