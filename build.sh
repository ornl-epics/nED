#!/bin/sh
cwd=`pwd`
prefix=BL100:Det2
controls=/home/controls/bl100
ioc=bl100-Det2-nED
stpath=${ioc}/iocBoot/ioc${ioc}/st.cmd
make clean
make
pushd pvtable
./create_ioc_pvtables.py -b ${prefix} -o ${controls}/pvtable/${ioc} -n ${cwd} ${controls}/applications/${stpath}
popd

