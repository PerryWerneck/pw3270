#!/usr/bin/env sh
#@author  André Breves <andre.breves@gmail.com>
set -euo

cd "${0%/*}"
executable_path="${PWD}"
cd ..
contents_path="${PWD}"
cd Resources
resource_path="${PWD}"

info_plist="${contents_path}/Info.plist"
bundle_identifier=$(plutil -extract "CFBundleIdentifier" xml1 -o - "${info_plist}" | sed -n "s/.*<string>\(.*\)<\/string>.*/\1/p")

# XDG Base Directory Specification
# https://specifications.freedesktop.org/basedir-spec/latest/ar01s03.html

# Defines the base directory relative to which user specific configuration files should be stored
export XDG_CONFIG_HOME="${HOME}/Library/Application Support/${bundle_identifier}"

# Defines the base directory relative to which user specific data files should be stored
export XDG_DATA_HOME="${HOME}/Library/Application Support/${bundle_identifier}"

# Defines the preference-ordered set of base directories to search for data files in addition to the $XDG_DATA_HOME base directory
export XDG_DATA_DIRS="${resource_path}"

# Defines the preference-ordered set of base directories to search for configuration files in addition to the $XDG_CONFIG_HOME base directory
export XDG_CONFIG_DIRS="${resource_path}"

# Defines the base directory relative to which user specific non-essential data files should be stored
export XDG_CACHE_HOME="${HOME}/Library/Caches/${bundle_identifier}"


# Running GTK+ Applications
# https://developer.gnome.org/gtk3/stable/gtk-running.html

# If set, makes GTK+ use $GTK_DATA_PREFIX instead of the prefix configured when GTK+ was compiled
export GTK_DATA_PREFIX="${resource_path}"

# Specifies the file listing the Input Method modules to load
export GTK_IM_MODULE_FILE="${resource_path}/gtk.immodules"

# Specifies the file listing the GdkPixbuf loader modules to load
export GDK_PIXBUF_MODULE_FILE="${resource_path}/gdk-loaders.cache"

# Specifies a list of directories to search when GTK+ is looking for dynamically loaded objects such as the modules
# specified by GTK_MODULES, theme engines, input method modules, file system backends and print backends.
export GTK_PATH="${contents_path}/Frameworks"

# Running GIO applications
# https://developer.gnome.org/gio/stable/running-gio-apps.html

# This variable can be set to the names of directories to consider when looking for compiled schemas for GSettings
export GSETTINGS_SCHEMA_DIR="${resource_path}"

# export LANG="pt_BR"
# export LC_MESSAGES="pt_BR"
# export LC_ALL="pt_BR"

mkdir -p "${XDG_CONFIG_HOME}"
mkdir -p "${XDG_DATA_HOME}"
mkdir -p "${XDG_CACHE_HOME}"

cd "${resource_path}"
exec "${executable_path}/pw3270"
