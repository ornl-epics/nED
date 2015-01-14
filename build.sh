#!/bin/sh
cwd=`pwd`
prefix=BL100:Det2
controls=/home/controls/bl100
stpath=bl100-Det2-nED/iocBoot/iocbl100-Det2-nED/st.cmd
make clean
pushd nedApp/src
make epicsdb
make
popd
pushd nedApp/Db
make
popd
exit 0
pushd pvtable
./instantiate.py -b ${prefix} -o ${controls}/opi ${controls}/applications/${stpath} DspPlugin_v*.pvst
./instantiate.py -b ${prefix} -o ${controls}/opi ${controls}/applications/${stpath} FemPlugin_v*.pvst
./instantiate.py -b ${prefix} -o ${controls}/opi ${controls}/applications/${stpath} RocPlugin_v*.pvst
popd

