
use arg uri

trace "?R"

host = .rx3270~new("pw3270:a")

say "PW3270 revision is "||host~revision()
say "Connection state is "||host~connected()
say "Ready state is "||host~ready()

if uri <> "URI"
	then say "Connect rc="||host~connect(uri)

say "Wait for ready is "||host~WaitForReady(60)

say "Text[3,2,27]="||host~GetTextAt(3,2,27)

say "ENTER exits with rc="||host~enter()

say "Wait for ready is "||host~WaitForReady(60)

return 0

::requires "rx3270.cls"

