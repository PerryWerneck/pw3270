# Maintainer: Perry Werneck <perry.werneck@gmail.com>

# SPDX-License-Identifier: LGPL-3.0-or-later 

# Copyright (C) 2024 Banco do Brasil S. A.
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

project(
	'pw3270', 
	['c'],
	version: '5.5.0',
	license: 'GPL-3.0-or-later',
)

project_description = 'IBM 3270 Terminal emulator for GTK'

scour = find_program('scour', required: true)

#
# Compiler flags
#
cc = meson.get_compiler('c')

enable_debug = get_option('debug') or get_option('buildtype').contains('debug')

time_epoch = run_command('sh', '-c', 'echo "$SOURCE_DATE_EPOCH"', check : true).stdout().strip()
if time_epoch == ''
  time_epoch = run_command('date','+%Y%m%d', check: true).stdout().strip()
  datecmd = 'date'
else
  datecmd = 'date -u -d@' + time_epoch
endif

compiler_flags = [
  '-ggdb3',
  '-ffat-lto-objects',
  '-fvisibility=hidden',
  '-pthread',
  '-fPIC',
  '-DHAVE_CONFIG_H=1',
  '-DBUILD_DATE='+time_epoch,
]

if enable_debug
  compiler_flags += [
	  '-DDEBUG=1'
  ]
else
  compiler_flags += [
	'-DNDEBUG=1'
  ]
endif

add_project_arguments(cc.get_supported_arguments(compiler_flags), language: 'c')

#
# Versioning
#
pkg_version = meson.project_version()
version_array = pkg_version.split('.')
pkg_major_version = version_array[0].to_int()
pkg_minor_version = version_array[1].to_int()
pkg_micro_version = version_array[2].to_int()

libversion = '@0@.@1@'.format(pkg_major_version,pkg_minor_version)

#
# i18n
#
subdir('po')

#
# Dependencies
#
lib3270 = dependency('lib3270')
lib_deps = [
  lib3270,
  dependency('libv3270'),
  dependency('glib-2.0'),
  dependency('gtk+-3.0'),
  dependency('gmodule-2.0'),
  dependency('gdk-3.0'),
]


#
# Configuration
#
app_conf = configuration_data()
app_conf.set_quoted('PACKAGE_NAME', meson.project_name())
app_conf.set('PRODUCT_NAME', lib3270.get_variable('product_name'))
app_conf.set('PRODUCT_ID', lib3270.get_variable('product_id'))
app_conf.set('PRODUCT_PATH', '/' + lib3270.get_variable('product_id').replace('.','/') + '/')
app_conf.set_quoted('PLUGIN_DIR',lib3270.get_variable('plugin_path'))
app_conf.set_quoted('PACKAGE_VERSION', meson.project_version())
app_conf.set_quoted('PACKAGE_DESCRIPTION', project_description)
app_conf.set('PACKAGE_VERSION_MAJOR', pkg_major_version)
app_conf.set('PACKAGE_VERSION_MINOR', pkg_minor_version)
app_conf.set('PACKAGE_VERSION_MICRO', pkg_micro_version)

app_conf.set('HAVE_LIBINTL', 1)
app_conf.set('HAVE_GNUC_VISIBILITY', 1)

rpq_timestamp_value = run_command('sh', '-c', datecmd + ' +%Y%m%d%H%M%S', check : true).stdout().strip()
app_conf.set_quoted('RPQ_TIMESTAMP_VALUE', rpq_timestamp_value)

package_release = run_command('sh', '-c', datecmd + ' +%-y.%-m.%-d', check : true).stdout().strip()
app_conf.set_quoted('PACKAGE_RELEASE', package_release)

package_release_date = run_command('sh', '-c', datecmd + ' +%Y-%m-%d', check : true).stdout().strip()
app_conf.set_quoted('PACKAGE_RELEASE_DATE', package_release_date)

if cc.compiles('#include <unistd.h>', name : 'unistd.h')
	app_conf.set('HAVE_UNISTD_H', 1)
endif

if host_machine.system() == 'windows'
	app_conf.set('DEFAULT_UI_STYLE', 0)
	app_conf.set('DEFAULT_MENUBAR_VISIBLE', 'true')
else
	app_conf.set('DEFAULT_UI_STYLE', 1)
	app_conf.set('DEFAULT_MENUBAR_VISIBLE', 'false')
endif

#
# Sources
#
includes_dir = include_directories(
    'src/include',
)

app_src = [
	'src/main/placeholders.c',
	'src/main/builder.c',
	'src/main/main.c',
	'src/main/tools.c',
	'src/objects/actions/abstract.c',
	'src/objects/actions/button.c',
	'src/objects/actions/clipboard.c',
	'src/objects/actions/dialog.c',
	'src/objects/actions/print.c',
	'src/objects/actions/save.c',
	'src/objects/actions/simple.c',
	'src/objects/actions/tools.c',
	'src/objects/actions/window.c',
	'src/objects/actions/view.c',
	'src/objects/application/actions/open.c',
	'src/objects/application/actions/preferences.c',
	'src/objects/application/actions/window.c',
	'src/objects/application/actions/about.c',
	'src/objects/application/open.c',
	'src/objects/application/application.c',
	'src/objects/application/plugins.c',
	'src/objects/keypad/attribute.c',
	'src/objects/keypad/element.c',
	'src/objects/keypad/load.c',
	'src/objects/keypad/model.c',
	'src/objects/keypad/widget.c',
	'src/objects/settings/actionview.c',
	'src/objects/settings/dialog.c',
	'src/objects/settings/widget.c',
	'src/objects/settings/gsettings.c',
	'src/objects/toolbar/actions.c',
	'src/objects/toolbar/models.c',
	'src/objects/toolbar/settings.c',
	'src/objects/toolbar/toolbar.c',
	'src/objects/window/actions/close.c',
	'src/objects/window/actions/connect.c',
	'src/objects/window/actions/filetransfer.c',
	'src/objects/window/actions/sessionproperties.c',
	'src/objects/window/header-settings.c',
	'src/objects/window/header.c',
	'src/objects/window/keyfile.c',
	'src/objects/window/page.c',
	'src/objects/window/terminal.c',
	'src/objects/window/tools.c',
	'src/objects/window/window.c',
]

if host_machine.system() == 'windows'
  app_src += [
	'src/main/windows/tools.c',
	'src/objects/os/windows/open.c',
	'src/objects/os/windows/savedesktopicon.c',
  ]
endif

if host_machine.system() == 'linux'
  app_src += [
	'src/main/linux/tools.c',
	'src/objects/os/linux/open.c',
	'src/objects/os/linux/savedesktopicon.c',
  ]
endif

if host_machine.system() == 'darwin'
  app_src += [
	'src/main/macos/tools.c',
	'src/objects/os/macos/open.c',
	'src/objects/os/macos/savedesktopicon.c',
  ]
endif

# Check for OSs
#https://mesonbuild.com/Reference-manual_builtin_host_machine.html

if host_machine.system() == 'windows'
  app_conf.set_quoted('LIBEXT', '.dll')
endif

if host_machine.system() == 'linux'
  app_conf.set_quoted('LIBEXT', '.so')  
endif

#
# Targets
#
config_src = [
  configure_file(
      output : 'config.h', 
      configuration : app_conf
  )
]

configure_file(
  input : 'schemas/application.gschema.xml.in',
  output : meson.project_name() + '-application.gschema.xml',
  install_dir: get_option('datadir') / 'glib-2.0/schemas',
  configuration : app_conf
)

configure_file(
  input : 'schemas/window.gschema.xml.in',
  output : meson.project_name() + '-window.gschema.xml',
  install_dir: get_option('datadir') / 'glib-2.0/schemas',
  configuration : app_conf
)

if host_machine.system() == 'windows'

  # https://mesonbuild.com/Windows-module.html
  windows = import('windows')
  resources = windows.compile_resources(
    configure_file(
      input : 'src/main/windows/resources.rc.in',
      output : 'resources.rc',
      configuration : app_conf
    )
  )

  executable(
	meson.project_name(),
	app_src + [ resources ],
	install: true,
	dependencies: lib_deps,
	include_directories: includes_dir
 )

else

  executable(
	meson.project_name(),
	app_src,
	install: true,
	dependencies: lib_deps,
	include_directories: includes_dir
 )

endif

configure_file(
	input : 'ui/' + host_machine.system() + '.ui.xml',
	output : lib3270.get_variable('product_name') + '.ui.xml',
	install_dir: get_option('datadir') + '/' + lib3270.get_variable('product_name'),
	configuration : app_conf
)

custom_target(
	'about-image',
	command: [ 
		scour,
		'../branding/' + meson.project_name() + '.svg',
		lib3270.get_variable('product_name') + '.svg',
	],
	output: lib3270.get_variable('product_name') + '.svg',
	install: true,
	install_tag: 'runtime',
	install_dir: get_option('datadir') + '/' + lib3270.get_variable('product_name'),
)

icons = [
	'connect.svg',          
	'disconnect.svg',           
	'gtk-connect-symbolic.svg',     
	'gtk-ok.svg',
	'gtk-quit.svg',           
	'ok.svg',   
	'quit.svg',
	'connect-symbolic.svg',  
	'disconnect-symbolic.svg',  
	'gtk-disconnect-symbolic.svg',  
	'gtk-ok-symbolic.svg',  
	'gtk-quit-symbolic.svg',  
	'ok-symbolic.svg',  
	'quit-symbolic.svg',
]

foreach icon : icons
	custom_target(
		icon,
		command: [ 
			scour,
			'../icons/' + icon,
			icon,
		],
		output: icon,
		install: true,
		install_tag: 'runtime',
		install_dir: get_option('datadir') + '/' + lib3270.get_variable('product_name') + '/icons',
	)
endforeach

if host_machine.system() == 'linux'

	configure_file(
		input : 'branding/launcher.desktop.in',
		output : lib3270.get_variable('product_id') + '.desktop',
		install_dir: get_option('datadir') / 'applications',
		install_tag: 'runtime',
		configuration : app_conf
	)

	custom_target(
		'app-icon-color',
		command: [ 
			scour,
			'../branding/' + meson.project_name() + '.svg',
			lib3270.get_variable('product_id') + '.svg',
		],
		output: lib3270.get_variable('product_id') + '.svg',
		install: true,
		install_tag: 'runtime',
		install_dir: get_option('datadir') + '/icons/hicolor/scalable/apps',
	)

	# TODO: How to save with same name??? Cant accept 'rename'
	custom_target(
		'app-icon-symbolic',
		command: [ 
			scour,
			'../branding/' + meson.project_name() + '-symbolic.svg',
			lib3270.get_variable('product_id') + '-symbolic.svg',
		],
		output: lib3270.get_variable('product_id') + '-symbolic.svg',
		install: true,
		install_tag: 'runtime',
		install_dir: get_option('datadir') + '/icons/hicolor/symbolic/apps'
	)

	configure_file(
		input : 'branding/metainfo.xml.in',
		output : lib3270.get_variable('product_id') + '.metainfo.xml',
		install_dir: get_option('datadir') / 'metainfo',
		configuration : app_conf
	)

	configure_file(
		input : 'branding/mime.xml.in',
		output : meson.project_name() + '.xml',
		install_dir: get_option('datadir') / 'mime/packages',
		configuration : app_conf
	)

endif
