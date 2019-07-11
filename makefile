SHELL = /bin/bash

# initial project structure
init:
	if ! [ -e obj ]; then mkdir obj; fi

.PHONY: init