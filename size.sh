#!/bin/bash

bold=$(tput bold 2>/dev/null)
red=$(tput setaf 1 2>/dev/null)
cyan=$(tput setaf 6 2>/dev/null)
normal=$(tput sgr0 2>/dev/null)

text=$(avr-size $1 | grep $1 | xargs | cut -d ' ' -f1)
data=$(avr-size $1 | grep $1 | xargs | cut -d ' ' -f2)
bss=$(avr-size $1 | grep $1 | xargs | cut -d ' ' -f3)

echo
echo "AVR Memory Usage:"
echo
echo -e "Program:  ${bold}${red}$(($text + $data))${normal} bytes (.text + .data)"
echo -e "Data:     ${bold}${cyan}$(($data + $bss))${normal} bytes (.data + .bss)"
echo
