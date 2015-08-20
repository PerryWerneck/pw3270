/*
 * Sample rexx code to get host charset
 */

 host = .rx3270~new("")

 say "Posição 19,39: "||host~getIsProtectedAt(19,39)
 say "Posição 20,39: "||host~getIsProtectedAt(20,39)

 return 0

::requires "rx3270.cls"
