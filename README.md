# RuuviAir Firmware Loader

The *RuuviAir Firmware Loader* implements a [Simple Management Protocol (SMP)](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/services/device_mgmt/mcumgr.html) server.  
It is derived from the `smp_svr` sample provided in the nRF Connect SDK:  
`ncs/v2.8.0/zephyr/samples/subsys/mgmt/mcumgr/smp_svr`.

## Purpose

The primary function of the Firmware Loader is to extend device management capabilities by supporting the following modules:

- **Image management (`img_mgmt`)**  
  Retrieve firmware version information.

- **File system management (`fs_mgmt`)**  
  Upload firmware update files to external flash memory using LittleFS.

## Integration

The Firmware Loader works together with the RuuviAir MCUboot bootloader to provide a complete secure firmware update path.
