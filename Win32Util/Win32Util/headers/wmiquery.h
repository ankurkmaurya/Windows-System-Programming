
#include <windows.h>
#include <iostream>
#include <comdef.h>
#include <Wbemidl.h>

#pragma once


class WMIQuery
{

public:

    /*
     *
      Options                -> option
      CPU                    -> cpu
      Logical Disk           -> logical.disk
      Disk Drive             -> disk.drive
      Disk Partition         -> disk.partition
      Physical Memory        -> physical.memory
      Computer System        -> computer.system
      Operating System (OS)  -> operating.system
      HotFixes               -> os.hot.fixes
      NetworkInterface       -> network.interface
      BIOS                   -> bios
      SystemUsers            -> system.users
    *
    */
    std::string getWMIQueryFromOption(char* option);

	void printWMIQueryResults(std::string wmiQuery);

	void readVTArrayData(VARIANT vtProp);
};



