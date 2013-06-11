/*
 * Sample rexx code to justified paste the clipboard contents.
 *
 * Autor: Perry Werneck <perry.werneck@gmail.com>
 *
 */

 host = .rx3270~new("")

 text = host~GetClipboard()
 if text = "" then
 do
    say "Clipboard is empty"
    return 0
 end

 say "["||text||"]"


return 0

::requires "rx3270.cls"



