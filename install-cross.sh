#!/bin/bash

PREFIX="mingw32"

TEMPFILE=$(mktemp)

cat > ${TEMPFILE} << EOF
atk-devel
pango-devel
win_iconv-devel
pixman-devel
glib2-devel
cairo-devel
freetype-devel
winpthreads-devel
gtk3-devel
libopenssl-devel
libintl-devel
cross-gcc-c++
cross-pkg-config
cross-cpp
cross-binutils
cross-nsis
filesystem
gettext-tools
gtk3-data
gtk3-tools
headers
gnome-icon-theme
hicolor-icon-theme
EOF

while read FILE
do
	sudo zypper in ${PREFIX}-${FILE}
done < ${TEMPFILE}

rm -f ${TEMPFILE}
