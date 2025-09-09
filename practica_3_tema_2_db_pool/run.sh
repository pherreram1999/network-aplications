#!/bin/bash

# in ubuntu install: sudo apt install libmariadb-dev libmariadb-dev-compat
gcc server.c  $(mysql_config --cflags --libs) -o server && source .env && ./server