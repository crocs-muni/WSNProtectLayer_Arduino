## WSNProtectLayer_Arduino

A port of WSNProtectLayer (https://github.com/crocs-muni/WSNProtectLayer) for (Arduino-compatible) JeeLink/JeeNode devices providing a subset of WSNProtectLayer's features.

#### Features:
* AES encryption and MAC for secure communication
* μTESLA authenticated broadcast
* Neighbor discovery
* Simplified CTP protocol
* Configurator for nodes' IDs and pairwise keys

### Building

The project is written in C++ and requires g++ to build Linux host applications and avr-gcc compiler to build JeeLink part. EduHoc (https://github.com/crocs-muni/Edu-hoc) project is also required to build and upload JeeLink applications.
_EDU_HOC_HOME_ variable needs to be set to EduHoc directory.
To build everything, run *make* in project directory:

```shell
make
```

After this, configurator host _config_host_ is located in _Configurator/host_ together with the library _libconfigurator.a_, base station host library _libcommon.a_ is located in _ProtectLayer/common_ and AES library _libaes.a_ in _ProtectLayer/common/AES_. Target directory of JeeLink applications is $EDU_HOC_HOME/bin/mini328/.
To use library in JeeLink devices, please see the demo applications in _ProtectLayer_ directory.

### Uploading

For now, JeeLink applications are uploaded by first version of JeeTool included in EduHoc project.

Example:
```shell
cd /path/to/this/project
java -cp /path/to/JeeTool cz.muni.fi.crocs.EduHoc.Main -a /file/with/JeeLinks/paths -u ProtectLayer/demo
```

This example uploads the demo application to nodes listed in /file/with/JeeLinks/paths.

### Components
The network consists of regular nodes and a single base station.
Base station consists of master running in Linux host and a slave as it requires more resources than a JeeLink device can provide. Slave device serves only as a radio.

### Project structure
_ProtectLayer_ directory contains base station slave (_BS_slave_ directory), all the library sources common for both Linux base station and JeeLink devices (_common_ directory) and 3 demo applications to present possible use cases.

_Configurator_ directory contains both Linux host application and JeeLink application to configure the device.

### Configuration

As stated above, _Configurator_ is used to configure devices. It takes an input file consisting of lines containing path to devices and their IDs. There is an example config file _config_ in _Configurator/host_.
Configurator can generate, save and upload keys to the JeeLink devices. Please run it with argument _-h_ to see the options.
To perform the configuration, a JeeLink part must be already uploaded and running in the devices.

## Licensing
The project uses AES implementation developed by Texas Instruments Incorporated under BSD-3-Clause license and some parts from original WSNProtectLayer licensed under BSD-2-Clause license.
Everything else is licensed under MIT license unless the specific file states otherwise.

