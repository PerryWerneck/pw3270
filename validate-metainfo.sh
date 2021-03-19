#!/bin/bash
./configure

cp branding/metainfo.xml /tmp/pw3270.metainfo.xml
appstream-util validate /tmp/pw3270.metainfo.xml

