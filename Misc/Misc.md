# Misc_2.c

1. Exported functions:

	- GWopenLibs(ULONG minVersion) - Opens required libraries:
		- Intuition,
		- GadTools,
		- Graphics,
		- IFFParse

	- GWcleanup() - Closes opened libraries
	- GWerror(STRPTR errorDesc) - Prints error string
	- GWbailout(STRPTR errorDesc) - Prints error string and exits

2. Notes

	This code is mainly to open libraries and setup basic system resources.
