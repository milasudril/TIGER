#!/bin/bash

array=(valgrind/*.supp)
VALGRIND_SUPPRESSIONS=${array[@]/#/--suppressions=}
echo $VALGRIND_SUPPRESSIONS
G_SLICE=always-malloc G_DEBUG=gc-friendly valgrind --track-origins=yes \
	$VALGRIND_SUPPRESSIONS --vgdb=no --vgdb-error=0 \
	__targets/tiger-ui

