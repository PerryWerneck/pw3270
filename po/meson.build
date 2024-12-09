# Maintainer: Perry Werneck <perry.werneck@gmail.com>

# SPDX-License-Identifier: LGPL-3.0-or-later 

# Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

#
# References:
#
# https://mesonbuild.com/i18n-module.html
# https://mesonbuild.com/Porting-from-autotools.html
# https://docs.elementary.io/develop/writing-apps/our-first-app/translations
#

# find 'src' -type f -iname '*.c' | grep -v 'build' | sed 's@^./@@g' | grep -v testprogram > po/POTFILES.in

i18n = import('i18n')

gettext_package = meson.project_name() + '-' + libversion

add_project_arguments('-DGETTEXT_PACKAGE="' + gettext_package + '"', language: 'c')

i18n.gettext(
	gettext_package, 
    args: '--directory=' + meson.project_source_root(),
    preset: 'glib'
)

