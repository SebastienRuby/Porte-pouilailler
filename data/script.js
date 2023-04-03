function openButton() {
	let xhttp = new XMLHttpRequest();
	xhttp.open("GET", "open", true);
	xhttp.send();
}

function closeButton() {
	let xhttp = new XMLHttpRequest();
	xhttp.open("GET", "close", true);
	xhttp.send();
}

setInterval(function getData()
{
	let xhttp = new XMLHttpRequest();

	xhttp.onreadystatechange = function()
	{
			if(this.readyState == 4 && this.status == 200)
			{
					document.getElementById("valeurPorte").innerHTML = this.responseText;
			}
	};

	xhttp.open("GET", "lireEtatPorte", true);
	xhttp.send();
}, 2000);