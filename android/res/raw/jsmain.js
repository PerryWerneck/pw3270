
function setupWindow()
{
	var input = document.getElementsByTagName("input");

	window.onkeypress = function()
	{
		if(event.keyCode == 13)
		{
			xmit();
			return false;
		}
		return true;
	}

	for(var i=0;i<input.length;i++)
	{
		if(input[i].name.substr(0,2) == "PF")
		{
			input[i].pfkey = parseInt(input[i].name.substr(3));

			input[i].onclick = function()
			{
				pw3270.pfkey(this.pfkey);
			}
		}

	}

}

function terminalUpdate()
{
	document.getElementById("terminal").innerHTML = pw3270.getscreencontents();
	setupWindow();
	pw3270.ready();
}

function pfkey(id)
{
	pw3270.busy();
	pw3270.pfkey(id);
	return false;
}

function xmit()
{
	var form = document.getElementById("form3270");

	pw3270.busy();

	if(form != undefined)
	{
		for(var i=0;i < form.elements.length;i++)
		{
			if(form.elements[i].name.substr(0,1) == "F")
			{
				var offset = parseInt(form.elements[i].name.substr(1),10);
				pw3270.setStringAt(offset,form.elements[i].value);
			}
		}
	}

	pw3270.sendEnter();

	return false;
}

function eraseinput()
{
	return false;
}
