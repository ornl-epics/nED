=== Files in this directory ===
There are several file types in this directory:
- *.db files are used as is and copied to the final EPICS db directory. No
  transformation is done, they may contain runtime replacable macros. In
  Makefile they appear with their name.

- *.template files are fed to 'msi' by make. Consult msi documentation for
  details. We use them to support Plugin class hierarchy and to hide
  dependencies to classes we derive from. In Makefile they appear with
  .template suffix changed to .db.

- *.substitution files are also used by 'msi' tool.

- *.include files can not be used directly. They need to be included in some
  .template file. They represent abstract C++ classes or automatically
  generated db files based on C++ code. They should not be included in
  Makefile.

=== Note on PV name length ===
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
