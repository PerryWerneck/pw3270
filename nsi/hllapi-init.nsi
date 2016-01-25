${GetParameters} $R0
ClearErrors
${GetOptions} $R0 /HLLAPI= $0

${if} $0 == "YES"

	SectionGetFlags "${HLLAPIPlugin}" $0
	IntOp $0 $0 | ${SF_SELECTED}
	SectionSetFlags "${HLLAPIPlugin}" $0

${EndIf}


