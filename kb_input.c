/*
 	set term to non cannon
	fcntl to not block on stdin reads
	atexit() to restore cannon mode

	function that process inputs ;
	each return is the input gotten ?
	then would loop over every call until -1 ?

	OR
	make function to register input handler void (int, void *)
	function call to process inputs waiting to be read (automatic call before or after every render ?)
	translation if needed (multi key keycodes ?)
	while (++input)
		handler(input)

	handler is stored in the display struct along with optionnal arg handler context

	should be automatic imo
	if no handler is registered, skip input detection phase
	-> should term cannon etc be set by setting input handler ? makes sense

	when should input processing be done ? before rendering a frame
	therefore render will call process inputs after outputting a frame
*/
/*
 	might be good to remove destroyDisplay and use atexit for that
*/



//old idea / alternative maybe
/*
fcntl can be used to emit signals when input or output is ready for a fd
stdin is a fd

SIGIO SIGURG

fcntl to send signal on fd change
sigaction for SIGIO or SIGPOLL to intercept it

fcntl fcsetown or ioctl call
 */
