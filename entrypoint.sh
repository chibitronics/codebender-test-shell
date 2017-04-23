#!/bin/sh
# Entry point for Love-to-Code web server frontend
#
# If the COMPILE_URL environment variable is set and nonzero,
# pass --compileUrl to the running program.
compile_url_arg=""
if [ ! -z "${COMPILE_URL}" ]
then
    compile_url_arg="--compileUrl ${COMPILE_URL}"
fi

exec gulp serve-prod ${compile_url_arg}