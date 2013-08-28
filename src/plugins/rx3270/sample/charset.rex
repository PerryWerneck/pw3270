/*
 * Sample rexx code to get host charset
 */

 host = .rx3270~new("")

 say "Display charset: "||host~getDisplayCharset()
 say "Host charset:    "||host~getHostCharset()

 return 0

::requires "rx3270.cls"
