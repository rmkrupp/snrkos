#!/bin/bash
# File: misc/convert-range.sh
# Part of snrkos <github.com/rmkrupp/snrkos>
# Original version from <github.com/rmkrupp/cards-client>
#
# Copyright (C) 2025 Noah Santer <n.ed.santer@gmail.com>
# Copyright (C) 2025 Rebecca Krupp <beka.krupp@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

if [ $# -lt 1 ] ; then
    echo "Syntax: $0 OUTPUT_SIZE"
    exit 1
fi

#
# TODO: this shouldn't always be square
# TODO: we should support creating multiple resolutions from each input
#

in=4096
out="$1"
spread="$1"
dir="$(mktemp -d)"
mkdir -p "$dir"
outdir="out/data/$out"
mkdir -p "$outdir"
template="data/template.svg"
function dfield() {
    echo generate "$1"
    sed 's/TEMPLATE/'"$1"'/' "$template" >"$dir/input.svg"
    inkscape -C -o "$dir/input.png" -w "$in" -h "$in" "$dir/input.svg" || exit 1
    magick "$dir/input.png" -transparent "#FFFFFFFF" -alpha Extract \
        "gray:$dir/input.dat" || exit 1
    ./tools/generate-dfield -I "$in" -O "$out" -S "$spread" \
        "$outdir/$1.dfield" "$dir/input.dat" || exit 1
    #magick -depth 8 -size "$out"x"$out" "gray:$outdir/$1.dfield" "$outdir/$1.png"
    rm "$dir/input.svg"
    rm "$dir/input.png"
    rm "$dir/input.dat"
}

#for i in \~ \` ! @ \# \$ % ^ \& '*' \(  \) _ - = + \[ \] \{ \} \| \\ : \; \' \" , "." \< \> / ? ; do dfield "$i" ; done
for i in {0..9} ; do dfield $i ; done
for i in {a..z} ; do dfield $i ; done
for i in {A..Z} ; do dfield $i ; done


