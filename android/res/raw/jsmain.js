
function initialize()
{
	updateScreen();
}

function updateScreen()
{
	document.getElementById("terminal").innerHTML = pw3270.getscreencontents();
}

function pfkey(id)
{
	pw3270.pfkey(id);
	return false;
}

function xmit()
{
	pw3270.xmit();
	return false;
}

function eraseinput()
{
	return false;
}

