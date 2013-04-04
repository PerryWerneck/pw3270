
use arg uri

host = .rx3270~new("")
host~ScreenTrace(1)

if host~connect(uri,1) <> 0 then
do
	say "Error connecting to "||uri
	return -1
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

