
use arg uri

host = .rx3270~new("")

if host~connect(uri) <> 0 then
do
	say "Erro ao conectar em "||uri
	return -1
end

host~sleep(20)

host~disconnect()

return 0


::requires "rx3270.cls"

