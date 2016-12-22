TEMPLATE = subdirs
SUBDIRS += src plugin
OTHER_FILES += rpm/libqofonoext.spec README
src.target = src-target
plugin.depends = src-target
