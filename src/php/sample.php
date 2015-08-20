<?php

	printf("PW3270 PHP sample\n");

	$host = new tn3270("pw3270:a");

	$rc = $host->connect();
	print("connect() exits with rc=" . $rc . "\n");

	$rc = $host->waitforready(10);
	print("waitforready() exits with rc=" . $rc . "\n");

	$str = $host->getstringat(3,2,14);
	print("Getstring(3,2,14) saiu com \"" . $str . "\"\n");

	$rc = $host->getisprotectedat(19,39);
	print("GetIsprotectedAt(19,39) saiu com \"" . $rc . "\"\n");

	$rc = $host->getisprotectedat(20,39);
	print("GetIsprotectedAt(20,39) saiu com \"" . $rc . "\"\n");

	$rc = $host->disconnect();
	print("disconnect() exits with rc=" . $rc . "\n");

?>
