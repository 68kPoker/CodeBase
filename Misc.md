# Misc

1. Misc_2.c/h

	a. Exported functions

		- GWopenLibs(ULONG minVersion) - Opens required libraries (Intuition, GadTools, Graphics, IFFParse)
		- GWcleanup() - Closes opened libraries
		- GWerror(STRPTR errorDesc) - Prints error string
		- GWbailout(STRPTR errorDesc) - Prints error string and exits

	b. Notes

		This code is mainly to open libraries and setup basic system resources.
