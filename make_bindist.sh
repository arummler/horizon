#!/bin/bash
DISTDIR=dist
rm -rf $DISTDIR
mkdir $DISTDIR
cp horizon-* $DISTDIR
strip $DISTDIR/horizon-*
LIBS=(
	libstdc++-6.dll\
	libgcc_s_seh-1.dll\
	libglibmm-2.4-1.dll\
	libglib-2.0-0.dll\
	libgio-2.0-0.dll\
	libgiomm-2.4-1.dll\
	libwinpthread-1.dll\
	libsqlite3-0.dll\
	libgmodule-2.0-0.dll\
	libgobject-2.0-0.dll\
	zlib1.dll\
	libintl-8.dll\
	libsigc-2.0-0.dll\
	libffi-6.dll\
	libiconv-2.dll\
	libpcre-1.dll\
	libatkmm-1.6-1.dll\
	libatk-1.0-0.dll\
	libgtk-3-0.dll\
	libgtkmm-3.0-1.dll\
	libpango-1.0-0.dll\
	libpangomm-1.4-1.dll\
	libcairomm-1.0-1.dll\
	libcairo-2.dll\
	libpangocairo-1.0-0.dll\
	libgdk-3-0.dll\
	libgdkmm-3.0-1.dll\
	libgdk_pixbuf-2.0-0.dll\
	libpangoft2-1.0-0.dll\
	libpangowin32-1.0-0.dll\
	libfontconfig-1.dll\
	libfreetype-6.dll\
	libcairo-gobject-2.dll\
	libepoxy-0.dll\
	libharfbuzz-0.dll\
	libpixman-1-0.dll\
	libpng16-16.dll\
	libexpat-1.dll\
	libbz2-1.dll\
	libgraphite2.dll\
	libjpeg-8.dll\
	librsvg-2-2.dll\
	libxml2-2.dll\
	liblzma-5.dll\
	libcroco-0.6-3.dll\
	libtiff-5.dll\
)
for LIB in "${LIBS[@]}"
do
   cp /mingw64/bin/$LIB $DISTDIR
done

mkdir -p $DISTDIR/share/icons
cp -r /mingw64/share/icons/Adwaita $DISTDIR/share/icons
cp -r /mingw64/share/icons/hicolor $DISTDIR/share/icons
rm -rf $DISTDIR/share/icons/Adwaita/cursors

mkdir -p $DISTDIR/lib
cp -r /mingw64/lib/gdk-pixbuf-2.0 $DISTDIR/lib
rm $DISTDIR/lib/gdk-pixbuf-*/*/loaders/*.a
