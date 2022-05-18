#!/bin/bash
pid=`pidof NQM_Controller_Linux`

if [ "$pid" ]; then
    kill -s SIGUSR1 $pid
fi

