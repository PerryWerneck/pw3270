/*
 * Sample code for EBCDIC -> ASC conversion
 *
 */

  STRING = x2c("C1E2C3C9C940E2E3D9C9D5C7")

  say length(string)
  say ebc2asc(STRING)

return 0


::requires "rx3270.cls"
