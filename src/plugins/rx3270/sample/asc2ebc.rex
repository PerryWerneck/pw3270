/*
 * Sample code for ASC -> EBCDIC conversion
 *
 */

 STRING = "ASCII STRING"

 say c2x(asc2ebc(STRING))

return 0


::requires "rx3270.cls"
