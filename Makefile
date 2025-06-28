# Uncomment lines below if you have problems with $PATH
#SHELL := /bin/bash
#PATH := /usr/local/bin:$(PATH)

all:
	platformio -c vim run

format:
	clang-format -style=file -i src/*

upload:
	platformio -c vim run --target upload

clean:
	platformio -c vim run --target clean

program:
	platformio -c vim run --target program

uploadfs:
	platformio -c vim run --target uploadfs

update:
	platformio -c vim update

monitor:
	platformio device monitor
