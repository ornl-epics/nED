#!/bin/env python

'''Load PV Table file,
   take snapshot,
   then save the file.
   
   Expects one PV for reporting progress 0..100

   Work in progress, not complete yet.
'''

from org.eclipse.jface.dialogs import MessageDialog
from org.eclipse.ui import PlatformUI
from org.eclipse.ui.part import FileEditorInput
from org.eclipse.core.resources import ResourcesPlugin
from org.eclipse.core.runtime import Path
from org.eclipse.core.runtime import NullProgressMonitor
from org.eclipse.swt.widgets import Display
from java.lang import Thread
from java.lang import Runnable

import os, sys, re, time

BL=os.environ['BL'].lower()
pv_table_path = "{0}/pvtable/{0}-Det-nED/".format(BL)
currentDisplay = Display.getCurrent()
n_total = 0
n_success = 0
n_fail = 0
progress_pv = pvs[1]

class ShowError(Runnable):
    """Runnable for displaying dialog off UI thread"""
    def __init__(self, message):
        self.message = message
    def run(self):
        MessageDialog.openInformation(None, "Error", self.message)


class SavePvtable(Runnable):
    """Runnable for handling one PV table"""
    def __init__(self, path):
        self.path = path

    def run(self):
        # Locate workspace file
        file = ResourcesPlugin.getWorkspace().getRoot().findMember(self.path)
        # Create editor input 
        input = FileEditorInput(file)
        # Open PV Table
        page = PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage()
        editor = page.openEditor(input, "org.csstudio.display.pvtable.ui.editor.PVTableEditor")
        
        model = editor.getModel()
        
        # TODO: need to put this in thread
        retries = 50
        while self.findDisconnected(model) > 0 and retries > 0:
            time.sleep(0.1)
            retries -= 1
        
        if retries > 0:
            # Get snapshot value of all checked items into model
            model.save()
        
            # Save to file
            editor.doSave(NullProgressMonitor())
    
        # Close editor, saving if there are changes
        page.closeEditor(editor, True)

        if retries == 0:
            MessageDialog.openInformation(None, "Error", "Table %s not saved, not all PVs connected" % self.path)

    # Didn't foresee an easy way to check from outside if all is connected
    def isConnected(self, item):
    #    if item.isComment():
    #        return True
        if item.getValue() == None:
            return False
        if item.getValue().getAlarmSeverity().name() == "UNDEFINED":
            return False
        return True
    
    def findDisconnected(self, model):
        disconnected = 0
        count = model.getItemCount()
        for i in range(count):
            item = model.getItem(i)
            if not self.isConnected(item):
                disconnected += 1
        return disconnected
    

class PvsSaver(Thread):
    def __init__(self, css_path):
        Thread.__init__(self)
        self._css_path = css_path.lstrip("/")
        
    def run(self):
        fs_path = os.path.join("/home/controls", self._css_path)
        files = os.listdir(fs_path)
        regex = re.compile("[a-zA-Z0-9_]_config\.pvs")
        paths = []
        config_files = []
        for file in files:
            if regex.search(file):
                config_files.append(file)
        
        n_total = len(config_files)
        n_current = 0
        for file in config_files:
            # Open Table, ... on the UI thread
            currentDisplay.syncExec(SavePvtable(os.path.join(self._css_path, file)))
            
            n_current += 1
            progress = 100.0 * n_current / n_total        
            progress_pv.setValue(progress)

# UI thread..
progress_pv.setValue(0.0)
thread = PvsSaver(pv_table_path)
thread.start()
