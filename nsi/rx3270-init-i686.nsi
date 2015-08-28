${if} ${FileExists} `$PROGRAMFILES\ooRexx\rexx.exe`

	SectionGetFlags "${RexxPlugin}" $0
	IntOp $0 $0 | ${SF_SELECTED}
	SectionSetFlags "${RexxPlugin}" $0

${EndIf}

