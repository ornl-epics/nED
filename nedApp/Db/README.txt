Note on PV name length
EPICS limits PV names to 29 characters including trailing zero byte. According
to SNS NamingStandard [1] some of the characters are reserved for identifying
beamline and area. In addition device type also takes up some characters.
Most of the records in this directory are just templates that can be used at
any beamline. Longest beamline prefix defines number of characters left for
register names. We've decided that 'BL99B:Det:aroc245:' should represent the
longest beamline PV prefix for nED, leaving us with 10 characters for register
names. For some beamlines even longer prefixes can be identified (ie. BNL ROC)
but those will have to be shortened to stay within 18 characters.

[1] https://trac.sns.gov/slowcontrols/wiki/NamingStandard
