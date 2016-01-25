${GetParameters} $R0
ClearErrors
${GetOptions} $R0 /JAVA= $0

${if} $0 == "YES"

	SectionGetFlags "${JNIModule}" $0
	IntOp $0 $0 | ${SF_SELECTED}
	SectionSetFlags "${JNIModule}" $0

${EndIf}


