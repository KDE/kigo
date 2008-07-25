#! /usr/bin/env bash
$EXTRACTRC `find -name \*.rc` >> rc.cpp || exit 11
$EXTRACTRC `find -name \*.ui` >> rc.cpp || exit 12
$XGETTEXT -kaliasLocal `find -name \*.cpp -o -name \*.h` rc.cpp -o $podir/kgo.pot
rm -f rc.cpp
