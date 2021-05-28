
	INCDIR	"Includes:"
	INCLUDE	"graphics/view.i"
	INCLUDE	"LVO/exec_lib.i"

	XDEF	_myCopper

_myCopper:

	MOVEA.L	(A1)+,A0
	MOVE.W	vp_Modes(A0),D0
	ANDI.W	#V_VP_HIDE,D0
	BNE.S	.skip

	MOVE.W	(A1)+,D1
	MOVEQ	#1,D0
	LSL.L	D1,D0
	MOVEA.L	(A1),A1
	MOVEA.L	4.W,A6
	JSR	_LVOSignal(A6)

.skip:
	MOVEQ	#0,D0
	RTS
