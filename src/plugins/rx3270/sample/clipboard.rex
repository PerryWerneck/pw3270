/*
 * Sample rexx code to justified paste the clipboard contents.
 *
 * Autor: Perry Werneck <perry.werneck@gmail.com>
 *
 */

 host = .rx3270~new("")

 if host~connected() = 0 then
 do
    say "Disconnected from host"
    return 0
 end

 text = host~GetClipboard()
 if text = "" then
 do
    say "Clipboard is empty"
    return 0
 end

 if host~WaitForReady(60) <> 0 then
 do
    say "Timeout waiting for host"
    return 0
 end

 cursor = host~GetCursorAddr()
 next   = cursor

 while text <> ""
 do
    addr = host~GetFieldStart(next)
    next = host~GetNextUnprotected(addr)

    host~SetCursorAddr(addr)

    field_len = host~GetFieldLen()

    if length(text) < field_len then
    do
        /* Text is smaller than field, just insert it */
        host~input(text)
    end
    else
    do
        /* Text is bigger than field, split ... */
        s = strip(left(text,field_len))
        p = lastpos(" ",s)
        if p = 0 then
        do
            s = strip(left(text,field_len))
            text = substr(text,field_len+1)
        end
        else
        do
            s = strip(left(text,p))
            text = strip(substr(text,p+1))
        end

        /* ... and justify */

        /* TODO */

        /* Insert new string */
        host~input(s)

    end

    say text

    if next <= cursor then
    do
        /* Next field is before the original position */
        leave
    end

    text = ""
 end

return 0

::requires "rx3270.cls"



