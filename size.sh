#!/bin/bash

text=$(avr-size $1 | grep $1 | xargs | cut -d ' ' -f1)
data=$(avr-size $1 | grep $1 | xargs | cut -d ' ' -f2)
bss=$(avr-size $1 | grep $1 | xargs | cut -d ' ' -f3)

echo
echo "AVR Memory Usage:"
echo
echo "Program:  $(($text + $data)) bytes (.text + .data)"
echo "Data:     $(($data + $bss)) bytes (.data + .bss)"
echo
