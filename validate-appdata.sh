#!/bin/bash
./configure

cp branding/appdata.xml /tmp/pw3270.appdata.xml
appstream-util validate /tmp/pw3270.appdata.xml

