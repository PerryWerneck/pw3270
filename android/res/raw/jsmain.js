
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
	var form = document.getElementById("form3270");

	for(var i=0;i < form.elements.length;i++)
	{
		if(form.elements[i].name.substr(0,1) == "F")
		{
			var offset = parseInt(form.elements[i].name.substr(1,4));
			pw3270.setStringAt(offset,form.elements[i].value);
		}
	}
	
	pw3270.sendEnter();
	
	return false;
}

function eraseinput()
{
	return false;
}
