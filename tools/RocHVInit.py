#!/bin/env python

import sys
import cothread
from cothread.catools import *

if len(sys.argv) < 2 :
   print 'USAGE:', sys.argv[0], '<PV prefix>'
   sys.exit()
prefix = sys.argv[1]

def transferValue(srcPV, destPV):
   inp = caget(prefix + srcPV)
   caput(prefix + destPV, inp)


transferValue('DConvX', 'DConvSetX')
transferValue('DConvY', 'DConvSetY')
transferValue('DConvZ', 'DConvSetZ')

transferValue('VConvX', 'VConvSetX')
transferValue('VConvY', 'VConvSetY')
transferValue('VConvZ', 'VConvSetZ')

transferValue('MaxV',   'MaxVSet')

transferValue('RampRate', 'RampRateSet')

caput(prefix + 'Volt.HIGH', 2300)
