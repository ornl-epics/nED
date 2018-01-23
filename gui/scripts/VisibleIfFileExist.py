import os.path

display_builder = 'getVersion' in dir(widget)
if display_builder:
    from org.csstudio.display.builder.runtime.script import PVUtil
else:
    from org.csstudio.opibuilder.scriptUtil import PVUtil

file = "/home/controls/" + PVUtil.getString(pvs[0])
widget.setVisible( os.path.isfile(file) )
