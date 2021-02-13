# Warehouse (Magazyn)
Sokoban Clone for Amiga

I have setup some classes in order to complete the game. Here's an overview of the general classes:
<!DOCTYPE html>
<html lang="pl">

<body>
<h3>CInput class</h3>
This document covers <b>CInput</b> - the class responsible on calling Input handlers for various event types:
<ul>
	<li>IDCMP - Mouse, Key events, Gadgets etc.;</li>
	<li>Joystick controller;</li>
	<li>Copper interrupt (animation synchronization);</li>
	<li>Safe to draw message.</li>
</ul>
<p>
Each Input handler takes the individual User data as well as <a href="CGame.html">CGame</a> space to manipulate on.
</p>
These root user datas are:
<ul>
<li><h4>CIDCMP</h4>It contains UserPort from which to Get Intuition Messages as well as User data;
<p>
	Methods:
	<ul>
		<li>initIDCMP() - Sets up CIDCMP with given UserPort.</li>
		<li>getMessage() - Gets Intuition Message from an CIDCMP UserPort.</li>
		<li>replyMessage() - Replies Intuition Message.</li>
	</ul></p></li>
	
<li><h4>CController</h4>It contains InputEvent from which to Get joystick state and IOStdReq from which to read next event;

<p>Methods:
<ul><li>initController() - Sets up CController with joystick IOStdReq/InputEvent.</li>
<li>readEvent() - Reads joystick InputEvent.</li>
</ul>

</p>
</li>
<li><h4>CCopper</h4>It contains data such as Signal, Task and ViewPort passed to Interrupt, as well as Interrupt structure;
<p>Methods:<ul><li>initCopper() - Sets up CCopper with given attributes (taken from <a href="CScreen.html">CScreen</a>).</li>
<li>waitCopper() - Waits for Copper interrupt - can be used outside handlers.</li><li>changeScreen() - Changes screen buffer. Must have access to CScreen and CSafeToDraw.</li></ul></p></li>

<li><h4>CSafeToDraw</h4>It contains Safe to Draw boolean switch and Message port.<p>
Methods:<ul><li>initSafeToDraw() - Inits CSafeToDraw with message port.</li>
<li>waitSafeToDraw() - Wait until it's safe to draw. This method may be called in other places, not only in handlers.
</li></ul></p></li>


</ul>

<h3>CScreen class</h3>

CScreen contains Screen, Screen Buffers and Windows structures. Each Window has associated CIDCMP and CInput.
Screen has associated CCopper and CSafeToDraw with CInputs for double-buffering and animation sync.

<p>Animation is done through CSafeToDraw and CCopper handlers and waitSafeToDraw() methods.
Proper RastPorts are attached to these structures via UserData pointers. Graphics data are associated with RastPorts.
<p>Gadgets are handled within CIDCMP handlers.
<ul>
<li><h4>CInputs</h4>It's a list of CInput records. The Signal masks are summed up.</li>
<li><h4>CWindow</h4>It contains Window and Gadgets pointers and CIDCMP + CInput.</li>
<li><h4>CGadget</h4>It contains Gadget handler that is called upon IDCMP_GADGETUP, IDCMP_GADGETDOWN and IDCMP_MOUSEMOVE.</li>
</ul>
<h3>CBoard class</h3>

CBoard contains CCells which have their own handler functions to - process and draw.
Board is associated with window on which it's drawn.

<h3>CIFF class</h3>

CIFF contains loader/saver for IFF files: ILBM, 8SVX, (later also SMUS), board data etc.
All chunk data is loaded using Handlers provided by iffparse.library.
</body>

</html>
