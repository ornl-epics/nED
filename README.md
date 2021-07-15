# nED - Neutron Event Distributor

## Introduction

nED is neutron data acquisition and detectors control software. It replaces dcomserver in functionality. It was designed from ground up to handle large amounts of data while providing stable operation. Name stands for neutron Event Distributor and was first suggested by Lisa M. Debeer-Schmitt around March 2014. nED development started in January 2014.

## Directory structure

Project is grouped into top level directories that can be further split into subdirectories. Directory structure follows EPICS application guidelines, in fact initial directory structure was create with makeBaseApp.pl EPICS tool.

* configure/
    This directory contains EPICS build files. Usually only RELEASE file needs to be updated when new EPICS dependency is required. Refer to EPICS documentation for details about this directory.
* iocned/
    Example IOC configuration is located here. Refer to section ioc_sec for details about instantiating nED IOC.
* nedApp/Db/
    nED EPICS database files are located here. Some of the files are generated dynamically from source files by export2epics.py script located in same directory and invoked automatically from Makefile. Refer to section plugin_params_sec for details how to create dynamic EPICS records.
* nedApp/doc/
    Doxygen configuration and support tools are located here. Doxygen is configured to look for sources in nedApp/src/. Some of the source code is parsed by export2doxy.py tool to extract EPICS records documentation. Any Doxygen supported output format can be built, at the moment HTML and PDF documentation is located in nedApp/doc/html/ and nedApp/doc/latex/ respectively.
* nedApp/src/
    nED C/C++ sources are located here. Every C++ class is defined in <class name>.h header file and implemented in <class name>.cpp file. Some of the detectors classes implement part of their functions in <class name>_v<detector version>.cpp files for easy parsing with external tools and comparing registers between different versions. EPICS sequencer files are named *.st. A CodeBlocks project file is provided for convenience, although CodeBlocks is not required to build or edit any of nED files.
* nedApp/src/unittest/
    Some of the core functionality is supported by unit tests. This includes FIFO implementation. Whenever code change is made to code covered with unit tests, those should be re-run to verify change correctness.
* protocol/
    This directory contains StreamDevice protocol files. Although main communication protocol with detectors is implemented in nED itself, some detectors are connected to external devices which support different protocol. For example, ROC boards can be connected to external high voltage devices which use ASCII protocol (ie. ?LV reads current voltage).
* tools/
    Support tools that are not specific to single directory are located here.

## Building

EPICS build system is engaged to build source code and to resolve dependencies. The top level Makefile file is used to build all required components. Some components (like IOC) are left out from automatic building but can be built manualy. When all dependant libaries listed in configure/RELEASE are installed, building nED should be as simple as running make in top level directory.

EPICS build system creates binaries with built in library search path. This means that all dependant libraries must not be moved to different location after nED was built.

Each of the top level subdirectories can be build individually.

## Architecture and design

nED designe is heavily influenced by areaDetector. Functionality is split into meaningful blocks called plugins. areaDetector calls them ports. Plugins can be attached to another plugin to receive detector data from and to send detector commands to. All plugins receive all detector data as DAS packets, but they only process those packets that they're supposed to. Some plugins are stackable, they process the data and serve it on to other plugins. There's exactly one plugin that connects to OCC hardware, it's called OccPortDriver.

Connecting plugins together is done in IOC startup file. This allows to modify nED configuration without modifying source code. Connecting plugins together can sometimes be done in more than one way and can provide performance optimizations. For example, in the graph above all detector plugins (DspPlugin, RocPlugin) are receiving data through CmdDispatcherPlugin to filter out packets that are not command responses. Since command responses are very infrequent comparing to data packets, this helps reduce CPU cycles by not invoking the plugin if there's no data for it. Especially with big number of detectors. Each plugin can be connected to one source plugin and many destination plugins. This can be described as parent/child relantionship, each plugin can have 1 parent but many children.

Each plugin can be individually configured to run in its own thread. When carefully configured, this provides further optimization improvements as plugins processing can be parallelized. See chapter Processing data for details.

## Plugins communication

All plugins are based on asynPortDriver class and they inherit all its capabilities. In asyn terminology an instance of asynPortDriver class is called port, but nED uses name plugin. We'll stick to plugin to not confuse the reader. asynPortDriver provides methods to register to other plugins, send data to registered plugins, receive outside get data callbacks and more. Sometimes detailed knowledge of asyn is required to understand some of the functionality. BasePlugin packs most often used capabilites and hides the complexity, providing a uniform API for derived plugins to use. Still similarities with asynPortDriver can be observed, so that experienced user can easily upgrade the provided functionality by matching it to asynPortDriver.

The most used communication mechanism between plugins is passing raw detector data between plugins. Detector data is formatted into packets called DAS packets. DasPacket class describes raw packet and provides several convenient functions and definitions for parsing packets. DasPackets are exchanged between plugins using the asyn's generic pointer interface. areaDetector uses same approach, but formats data into NDArray structure. nED would have to convert raw data into NDArray before sending to plugins and back to DasPacket format in plugins. Passing DasPackets is done very efficiently with zero-copy all the way to actually sending data to external clients. When OCC board receives data it puts it into system memory using DMA. nED is notified about the location of new data and casts it as DasPacket. As DasPacket is pointer container it doesn't copy the data. DasPackets are grouped into DasPacketList and sent to plugins using the asyn generic pointer interface. Plugin is notified by calling its overloaded proecessData() function. DasPacket data must not be altered by plugins, because all plugins receive pointer to the same memory. When plugins wants to send data to detectors, it packs it into DasPacket container and sends it to parent plugin. The operation itself is synchronous and handle is given back as soon as data reaches OCC board. But actual response is asynchronous, response to the command just sent is received when detector sends it through the receive mechanism described above.

Plugins so far don't use other asyn types (asynInt32, asynFloat64, etc.) for inter-communication but they could. However all EPICS database records use these basic types for getting or setting plugin parameters. Consult asynPortDriver documentation for details. One thing to remember is that none of the plugin callbacks must block, meaning wait undeterministically. Usually this involves writing to files, sockets etc.
