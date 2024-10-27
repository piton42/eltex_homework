#!/bin/bash

for i in {1..20}; do
 mkdir -p "$i"
 touch "$i/$i.txt"
 ln -s "$i/$i.txt" ./"$i.txt"
 echo "$i/$i.txt" > "$i/$i.txt"
done

# Удаление файлов, не содержащих "4" 
 find . -mindepth 1 -type f -exec sh -c '
  grep -q 4 "$1" || rm "$1"
 ' sh {} \;

echo "Новое содержимое 14/14.txt" > "14/14.txt"
