import epics
import pvaccess
import re
import os

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
    pv = _pvprefix + plugin + ":" + param
    if _verbose:
        print "nED get PV: ", pv
    return epics.get_pv(pv, connect=wait_connect, timeout=timeout)

def getPva(name="Neutrons"):
    return pvaccess.Channel( _pvprefix + "pva:" + name.strip(":") )

def getModuleNames(expr=".*"):
    global _verbose

    try:
        o = pvaccess.Channel(_pvprefix + "pva:Modules").get()
        modules = o.getScalarArray('modules.value')
        if not modules:
            modules = []
    except:
        modules = []

    # Convert simple globing to regular expression
    expr = re.sub(r"([^\.]?)\*", r"\1.*", expr)

    # Select only modules user cares about
    regex = re.compile(expr)
    modules = list(filter(lambda x: regex.match(x), modules))

    modules = sorted(modules, key=lambda s: s.lower())

    if _verbose:
        print "Discovered modules: ", modules

    return modules

def getModules(expr=".*"):
    modules = []
    for module in getModuleNames(expr):
        modules.append(Module(module))
    return modules

class Module:
    def __init__(self, name):
        self.name = name
        self.__dict__['name'] = name
        self._req_pv = None
        self._rsp_pv = None

        self._cached_pvs = []
        for pv in [ "HwId", "Type", "FwVerStr.SVAL", "Status", "StatusText", "OutputMode" ]:
            getPv(name, pv, wait_connect=False)
            self._cached_pvs.append(pv)

    def __getattr__(self, param):
        pv = self.getPv(param)
        if not pv:
            return None
        return pv.get()

    def getPv(self, param):
        global _verbose

        # User is likely asking for one of the registers and is likely to ask
        # for others in the same group of 'Config', 'Status', etc.
        # Let's start to connect them all to minimize the connection delay
        if param not in self._cached_pvs:
            for group in [ "Status", "Config", "Temp", "Counter" ]:
                params = self.getParams(group)
                if param in params:
                    if _verbose:
                        print "Caching '{0}' parameters".format(group)
                    for p in params:
                        getPv(self.name, p, wait_connect=False)
                    self._cached_pvs += params
                    break

        # Now select the requested one and make sure it's connected
        return getPv(self.name, param, wait_connect=True)

    def getParams(self, typ=None):
        if typ:
            return self._getParams(typ)

        params  = self._getParams("Status")
        params += self._getParams("Config")
        params += self._getParams("Temp")
        params += self._getParams("Counter")
        return params

    def _getParams(self, typ=None):
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

    @staticmethod
    def sendCommand(name, command):
        if isinstance(name, Module):
            self = name
            if self._req_pv is None:
                self._req_pv = getPv(self.name, "CmdReq")
                self._rsp_pv = getPv(self.name, "CmdRsp")
            req_pv = self._req_pv
            rsp_pv = self._rsp_pv
        else:
            req_pv = getPv(name, "CmdReq")
            rsp_pv = getPv(name, "CmdRsp")

        req_pv.put(command)
        while rsp_pv.get(as_string=True) == "Waiting":
            time.sleep(0.01)
        return rsp_pv.get(as_string=True) != "Success"
