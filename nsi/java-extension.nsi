		Section /o "Java" JNIModule

			setOutPath $INSTDIR
			file "/oname=$SYSDIR\jni3270.dll"			"..\.bin\Release\jni3270.dll"
			file "/oname=$INSTDIR\plugins\j3270.dll"	"..\.bin\Release\plugins\j3270.dll"

		sectionEnd

