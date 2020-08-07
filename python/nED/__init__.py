import epics
import pvaccess
import re
import os
import time

BL=os.environ['BL'] if "BL" in os.environ else "BL100"
_pvprefix = os.environ['BL'] + ":Det:"
_pvs = {}
_verbose = False

def setVerbose(verbose):
    global _verbose
    _verbose = verbose

def getPvPrefix():
    global _pvprefix
    return _pvprefix

def setPvPrefix(prefix):
    global _pvprefix
    if not prefix.endswith(":"):
        prefix += ":"
    _pvprefix = prefix

def getPv(plugin, param, wait_connect=True, timeout=1.0):
    """Returns PyEpics PV object for the selected plugin parameter"""
    global _pvprefix
    global _verbose
    # We use epics.get_pv() as it caches PVs and they don't have to reconnect every time
    # get_pv() also doesn't create new PV object except the first time => faster
    pv = getPvName(plugin, param)
    if _verbose:
        print "nED get PV: ", pv
    return epics.get_pv(pv, connect=wait_connect, timeout=timeout)

def getPvName(plugin, param):
    return _pvprefix + plugin + ":" + param

def getDataChannel(event_type="Pixel"):
    """ Return a new not-connected instance of PVA channel """
    global autorestore

    # Get PV channel name based on event type
    event_type = event_type.strip(":").title()
    pvaName = getPv("pva", event_type + "PvaName").get()
    ch = pvaccess.Channel(pvaName)

    # Now make sure this channel is enabled, but keep it at its previous
    # settings after we're done with this program
    enablePv = getPv("pva", event_type + "Enable")
    autorestore.save(enablePv)
    enablePv.put(1)

    return ch

def getModuleNames(expr=".*"):
    """Return a list of module names that match the pattern expression."""
    global _verbose

    try:
        o = pvaccess.Channel(_pvprefix + "pva:Modules").get("")
        modules = o.getScalarArray('modules.value')
    except:
        raise RuntimeError("Failed to communicate to nED")

    # Convert simple globing to regular expression
    expr = "^" + re.sub(r"([^\.]?)\*", r"\1.*", expr) + "$"

    # Select only modules user cares about
    regex = re.compile(expr)
    modules = list(filter(lambda x: regex.match(x), modules))

    modules = sorted(modules, key=lambda s: s.lower())

    if _verbose:
        print "Discovered modules: ", modules

    return modules

def getModules(expr=".*"):
    """Return a list of modules that match the pattern expression.

    The returned list contains Module instances and since it's connecting
    PVs automatically, this function is slower than getModuleNames() if
    you don't need all Module class functionality.
    """
    modules = []
    for module in getModuleNames(expr):
        modules.append(Module(module))
    return modules

class Module:
    def __init__(self, name, cache_params=True):
        self.name = name
        self.__dict__['name'] = name
        self._req_pv = None
        self._rsp_pv = None
        self._params = None

        self._cached_pvs = []
        for pv in [ "HwId", "HwType", "FwVersion", "Status", "StatusText", "OutputMode" ]:
            getPv(name, pv, wait_connect=False)
            self._cached_pvs.append(pv)

        if cache_params:
            self.getParams()

    def __getattr__(self, param):
        if param.startswith("_"):
            raise AttributeError("Module class has no attribute {0}".format(param))
        pv = self.getPv(param)
        if "enum" in pv.type:
            return pv.get(as_string=True)
        return pv.get()

    def getPv(self, param, auto_restore=False):
        global _verbose
        global autorestore

        # User is likely asking for one of the registers and is likely to ask
        # for others in the same group of 'Config', 'Status', etc.
        # Let's start to connect them all to minimize the connection delay
        if param not in self._cached_pvs:
            self._cached_pvs = self.getParams()
            if _verbose:
                print "Caching all parameters"
            for p in self._cached_pvs:
                getPv(self.name, p, wait_connect=False)

        # Now select the requested one and make sure it's connected
        pv = getPv(self.name, param, wait_connect=True)
        if pv and auto_restore:
            autorestore.save(pv)
        return pv

    def getPvName(self, param):
        return getPvName(self.name, param)

    def getParams(self, typ=None):
        """ Returns a list of module parameters.

        Module parameters in nED are grouped based on 4 types:
        config, status, temperature, counter.
        `typ' argument can be used to select only parameters from one of those
        4 groups.
        """
        if  self._params is None:
            # Late initialization
            self._params = {}
            try:
                o = pvaccess.Channel(_pvprefix + "pva:" + name + ":Params").get("")
                self._params['config'] = o.getScalarArray('config.value')
                self._params['status'] = o.getScalarArray('status.value')
                self._params['counter'] = o.getScalarArray('counter.value')
                self._params['temperature'] = o.getScalarArray('temperature.value')
            except:
                self._params['config'] = self._getParamsFromFiles('config')
                self._params['status'] = self._getParamsFromFiles('status')
                self._params['counter'] = self._getParamsFromFiles('counter')
                self._params['temperature'] = self._getParamsFromFiles('temperature')

        if typ is not None:
            if typ not in [ 'config', 'status', 'counter', 'temperature' ]:
                raise UserWarning("Invalid module parameter group `%s'".format(typ))
            params  = self._params[typ]
        else:
            try:
                params  = self._params["status"]
                params += self._params["config"]
                params += self._params["temperature"]
                params += self._params["counter"]
            except KeyError:
                raise UserWarning("Failed to initialize module parameters")
        return params

    def _getParamsFromFiles(self, typ=None):
        remap = {
            "config": "Config",
            "status": "Status",
            "counter": "Counters",
            "temperature": "Temp"
        }
        if typ in remap:
            typ = remap[typ]

        pluginid = getPv(self.name, "PluginId").get(as_string=True)
        params = []
        top = os.path.dirname(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))
        src = os.path.join(top, "nedApp", "src", pluginid + ".cpp")
        with open(src, "r") as f:
            regex = re.compile(".*create[a-zA-Z]*" + typ.title() + "Param\(\"([^\"]*)\"")
            for line in f.readlines():
                m = regex.match(line)
                if m:
                    params.append(m.group(1))
        return params

    def sendCommand(self, command):
        if self._req_pv is None:
            self._req_pv = self.getPv("CmdReq")
            self._rsp_pv = self.getPv("CmdRsp")

        self._req_pv.put(command, wait=True, timeout=10)
        return self._req_pv.severity == 0 and self._rsp_pv.get(as_string=True) == "Success"

    def saveConfig(self, name):
        pv = getPv(self.name, "SaveConfig")
        pv.put(name, wait=True)
        if pv.severity != 0:
            raise UserWarning("Failed to save {0} configuration '{1}'".format(self.name, name))

    def restoreConfig(self, name):
        pv = getPv(self.name, "LoadConfig")
        pv.put(name, wait=True)
        if pv.severity != 0:
            raise UserWarning("Failed to restore {0} configuration '{1}'".format(self.name, name))

    def applyConfig(self, retries=3):
        if self.getPv("Acquiring").get() == 1:
            cmds = [ "Stop acquisition", "Write config", "Start acquisition" ]
        else:
            cmds = [ "Write config" ]
        for cmd in cmds:
            for retry in range(retries):
                if self.sendCommand(cmd):
                    break
            else:
                raise UserWarning("Failed to apply configuration, command '{0}' failed".format(cmd))

    def listConfigs(self):
        return self.getPv("LoadConfigMenu").enum_strs

class _AutoRestore:
    """ This class is used for automatically restoring PV values when program exits.

    In its destructor, this class will restore all previously saved values.
    This is useful for some functionality that needs to be enabled temporarily
    while program is running and would impact performance otherwise. Once PV and
    its value is saved, it can be then changed to whatever value and this class
    will ensure it gets reverted back.

    Due to use of __del__() destructor, auto-restore method is not 100% reliable.
    """
    def __init__(self):
        self._pvs = {}

    def __del__(self):
        for pv,value in self._pvs.iteritems():
            pv.put(value)

    def save(self, pv, restore_value=None, init_only=True):
        """ Saves a value to be restored on shutdown.

        If restore_value is not provided, current value from PV is used instead.
        This method can be called many times for same PV with new values if so
        desired.
        """
        if not init_only or pv not in self._pvs:
            if restore_value is None:
                restore_value = pv.get()
            self._pvs[pv] = restore_value
autorestore = _AutoRestore()
