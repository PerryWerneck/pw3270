/*
 * Sample rexx code to justified paste the clipboard contents.
 *
 * Autor: Perry Werneck <perry.werneck@gmail.com>
 *
 */

 host = .rx3270~new("pw3270:a")

 if host~connected() = 0 then
 do
    host~popup("error","Can't start script","Disconnected from host")
    return 0
 end

 text = strip(host~GetClipboard())
 if text = "" then
 do
    say "Clipboard is empty"
    return 0
 end

 if host~WaitForReady(60) <> 0 then
 do
    host~popup("error","Failed","Timeout waiting for host response")
    return 0
 end

 cursor = host~GetCursorAddr()
 next   = cursor

 do while text <> ""
    addr = host~GetFieldStart(next)
    next = host~GetNextUnprotected(addr)

    host~SetCursorAddr(addr)

    field_len = host~GetFieldLen()

    s = strip(left(text,field_len))
    p = lastpos(" ",s)
    n = pos(d2c(10),s)

    select
    when n <> 0 then
    do
        s = strip(left(text,n-1))
        text = strip(substr(text,n+1))
    end

    when length(text) < field_len then
    do
        s = justify(strip(text),field_len)
        text = ""
    end

    when p = 0 then
    do
        s = strip(left(text,field_len))
        text = substr(text,field_len+1)
    end

    otherwise
        s = strip(left(text,p))
        text = strip(substr(text,p+1))
    end

    /* Insert new string */
    host~input(s)

    if next <= cursor then
    do
        /* Next field is before the original position */
        host~SetClipboard(text)
        return 0
    end

 end

return 0

justify: procedure

    use arg text, len

    wlen = words(text)
    if wlen < 3
        then return text

    ln = .array~new()
    sz = 0

    do f = 1 to wlen
        ln[f] = word(text,f)||" "
        sz  = sz + length(ln[f])
    end

    do while sz < len
        do f = wlen-1 to 1 by -1
            ln[f] = ln[f]||"."
            sz = sz+1
            if sz >= len
                then leave
        end
    end

    str = ""
    do f = 1 to wlen
        str = str||ln[f]
    end

return strip(str)

::requires "rx3270.cls"



