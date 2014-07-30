#!/bin/bash
killall -9 DFSystem DFS_DataClient DFS_DataServer
mv Video.lst.Ready Video.lst
mv Video.lst.Finish Video.lst
rm -f 0.* 1.*
make clean all
./DFS_DataClient
