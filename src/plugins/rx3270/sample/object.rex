
use arg uri

if arg(1) <> "" then
do

	/* Has a host URI, create a new session and connect to the host */

	host = .rx3270~new("")

	if host~connect(uri,1) <> 0 then
	do
		say "Error connecting to "||uri
		return -1
	end
end
else
do
	/* No host URI, use the first session */
	host = .rx3270~new("pw3270:A")

end

if host~WaitForReady(60) <> 0 then
do
	say "Timeout waiting for terminal ready"
end

say "Text(2,2,23)=["||host~GetTextAt(2,2,23)||"]"
say "Text(3,2,27)=["||host~GetTextAt(3,2,27)||"]"

host~enter()
if host~WaitForReady(60) <> 0 then
do
	say "Timeout waiting for terminal ready"
end

host~disconnect()

return 0


::requires "rx3270.cls"

