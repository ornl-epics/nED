import epics
import pvaccess
import re
import os

BL=os.environ['BL'] if "BL" in os.environ else "BL100"
_pvprefix = os.environ['BL'] + ":Det:"
_pvs = {}

def getPvPrefix():
    global _pvprefix
    return _pvprefix

def setPvPrefix(prefix):
    global _pvprefix
    if not prefix.endswith(":"):
        prefix += ":"
    _pvprefix = prefix

def getPv(plugin, param, connection_timeout=1.0):
    """Returns PyEpics PV object for the selected plugin parameter"""
    global _pvprefix
    # We use epics.get_pv() as it caches PVs and they don't have to reconnect every time
    # get_pv() also doesn't create new PV object except the first time => faster
    pv = _pvprefix + plugin + ":" + param
    return epics.get_pv(pv, connection_timeout=connection_timeout)

def getModuleNames(expr=".*"):
    getPv("modules", "RefreshPvaList").put(1)
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

    return sorted(modules, key=lambda s: s.lower())

def getModules(expr=".*"):
    modules = []
    for module in getModuleNames(expr):
        modules.append(Module(module))
    return modules

class Module(epics.device.Device):
    def __init__(self, name):
        global _pvprefix
        epics.device.Device.__init__(self, _pvprefix + name + ":")
        self.__dict__['name'] = name

        self.add_pv(_pvprefix + name + ":HwId",          attr="address")
        self.add_pv(_pvprefix + name + ":FwVerStr.SVAL", attr="version")
        self.add_pv(_pvprefix + name + ":FwDate",        attr="date")
        self.add_pv(_pvprefix + name + ":StatusText",    attr="status")
        self.add_pv(_pvprefix + name + ":PluginId",      attr="pluginid")

        # Handle enums as strings
        self.__dict__['type']     = getPv(name, "HwType").get(as_string=True)
        self.__dict__['datamode'] = getPv(name, "OutputMode").get(as_string=True)

        self.req_pv = None
        self.rsp_pv = None

    def getParams(self, typ):
        params = []
        top = os.path.dirname(os.path.dirname(os.path.dirname(__file__)))
        src = os.path.join(top, "nedApp", "src", self.pluginid + ".cpp")
        with open(src, "r") as f:
            regex = re.compile(".*create[a-zA-Z]*" + typ.title() + "Param\(\"([^\"]*)\"")
            for line in f.readlines():
                m = regex.match(line)
                if m:
                    params.append(m.group(1))
        return params

    @staticmethod
    def sendCommand(self, command):
        if isinstance(self, Module):
            if req is None:
                self.req_pv = getPv(self.__dict__['name'] + ":CmdReq")
                self.rsp_pv = getPv(self.__dict__['name'] + ":CmdRsp")
            req = self.req_pv
            rsp = self.rsp_pv
        else:
            self.req_pv = getPv(name + ":CmdReq")
            self.rsp_pv = getPv(name + ":CmdRsp")

        req.put(command)
        rsp = getPv(name, "CmdRsp")
        while rsp.get(as_string=True) == "Waiting"):
            time.sleep(0.01)
        return rsp.get(as_string=True) != "Success"

    @staticmethod
    def sendConfig(self):
        name = self.__dict__['name'] if isinstance(self, Module) else self

        req = getPv(name, "CmdReq")
        rsp = getPv(name, "CmdRsp")

        acquiring = (getPv(name, "Acquiring").get(as_string=True) == "acquiring")
        if acquiring:
            if not sendCommand(self, "Stop acquiring"):
                return False
