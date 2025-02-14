#!/bin/bash

gcc main.c server.c core.c arguments.c pool.c -lm -o dz_server && ./dz_server
