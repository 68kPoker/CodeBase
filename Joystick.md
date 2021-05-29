# Joystick

1. Joystick_2.c/h

	a. Exported functions

		- IOStd openJoy(struct InputEvent *ie) - sets up joystick
		- void closeJoy(IOStd io) - Closes joystick
		- void readEvent(IOStd io, struct InputEvent *ie) - reads next joystick event

	b. Notes

		This code is 100% complete and working.

