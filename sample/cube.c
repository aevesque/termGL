#include "../include/termGL.h"

typedef struct {
	volatile int	dir;
	volatile int	should_quit;
} s_context;

void	handler(char code, void *context)
{
	if (code == 32)
		((s_context *)context)->dir *= -1;
	if (code == 27)
		((s_context *)context)->should_quit = 1;
}

static void	renderCube(const int framerate, const float rpm, s_context *context, TermGL termGL)
{
	float	angle = 0;
	const float	angle_step = 360.0f  / (framerate * 60.0f) * rpm;
	const unsigned int	display_height = getDisplayHeight(termGL);

	fVec3	p0 = {-0.4, 0.4, -0.4};
	fVec3	p1 = {0.4, 0.4, -0.4};
	fVec3	p2 = {-0.4, -0.4, -0.4};
	fVec3	p3 = {0.4, -0.4, -0.4};

	fVec3	p4 = {-0.4, 0.4, 0.4};
	fVec3	p5 = {0.4, 0.4, 0.4};
	fVec3	p6 = {-0.4, -0.4, 0.4};
	fVec3	p7 = {0.4, -0.4, 0.4};

	while (!context->should_quit)
	{
		angle += angle_step * context->dir;

		uintVec3	rp0 = toAbsolute(rotateY(p0, angle), DISPLAY(termGL));
		uintVec3	rp1 = toAbsolute(rotateY(p1, angle), DISPLAY(termGL));
		uintVec3	rp2 = toAbsolute(rotateY(p2, angle), DISPLAY(termGL));
		uintVec3	rp3 = toAbsolute(rotateY(p3, angle), DISPLAY(termGL));

		uintVec3	rp4 = toAbsolute(rotateY(p4, angle), DISPLAY(termGL));
		uintVec3	rp5 = toAbsolute(rotateY(p5, angle), DISPLAY(termGL));
		uintVec3	rp6 = toAbsolute(rotateY(p6, angle), DISPLAY(termGL));
		uintVec3	rp7 = toAbsolute(rotateY(p7, angle), DISPLAY(termGL));

		drawLine(rp0, rp1, BLUE, DISPLAY(termGL));
		drawLine(rp1, rp3, BLUE, DISPLAY(termGL));
		drawLine(rp2, rp3, BLUE, DISPLAY(termGL));
		drawLine(rp2, rp0, BLUE, DISPLAY(termGL));

		drawLine(rp4, rp5, GREEN, DISPLAY(termGL));
		drawLine(rp4, rp6, GREEN, DISPLAY(termGL));
		drawLine(rp7, rp5, GREEN, DISPLAY(termGL));
		drawLine(rp7, rp6, GREEN, DISPLAY(termGL));

		drawLine(rp0, rp4, RED, DISPLAY(termGL));
		drawLine(rp2, rp6, RED, DISPLAY(termGL));

		drawLine(rp1, rp5, RED, DISPLAY(termGL));
		drawLine(rp3, rp7, RED, DISPLAY(termGL));

		putText("Press spc to change direction", 0, display_height - 4, WHITE, BLACK, DISPLAY(termGL));
		putText("Esc to quit", 0, display_height - 2, BLACK, WHITE, DISPLAY(termGL));
		renderDisplay(termGL);
	}
}

int	main(void)
{
	s_context context = {
		.dir = 1,
	};
	const int framerate = 60;
	const float rpm = 40;

	TermGL	termGL = termGLInit(50, 50);

	setFramerate(framerate, termGL);
	registerInputHandler(handler, &context, termGL);
	renderCube(framerate, rpm, &context, termGL);
	termGLDestroy(termGL);
}
