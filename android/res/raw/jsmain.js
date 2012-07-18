
function setupButtons()
{
	var form = document.getElementById("form3270");

	if(form == undefined)
		return;

	for(var i=0;i < form.elements.length;i++)
	{
		if(form.elements[i].name.substr(0,2) == "PF")
		{
			form.elements[i].pfkey = parseInt(form.elements[i].name.substr(3));
			
			form.elements[i].onclick = function()
			{
				pw3270.pfkey(this.pfkey);
			}
		}
	}

}

function initialize()
{
	updateScreen();
}

function updateScreen()
{
	document.getElementById("terminal").innerHTML = pw3270.getscreencontents();
	setupButtons();
}

function pfkey(id)
{
	pw3270.pfkey(id);
	return false;
}

function xmit()
{
	var form = document.getElementById("form3270");

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
