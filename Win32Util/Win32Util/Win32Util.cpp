
#include <cstring>
#include <wmiquery.h>
#include <telnet.h>
#include <ping.h>
#include <servicescm.h>


int main(int argc, char* argv[])
{
    //std::cout << "Argument count: " << argc << "\n";
    char* command = NULL;
    char* option = NULL;

    char* ipAddress = NULL;
    char* port = NULL;

    char* serviceQueryFormat = NULL;
    char* serviceName = NULL;
    char* serviceDisplayName = NULL;
    char* serviceDescription = NULL;
    char* serviceBinaryPath = NULL;
    char* serviceStartType = NULL;


    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-help") == 0)
        {
            std::cout << "Usage Guide:" << std::endl;
            std::cout << "Win32Util [command] [options]" << std::endl;
            std::cout << "Note : Argument containing spaces must be quoted." << std::endl;
            std::cout << std::endl;

            std::cout << "Commands Available - " << std::endl;
            std::cout << "1. wmi      : Provides system information." << std::endl;
            std::cout << "2. telnet   : Provides functionality to test connectivity with IP and Port." << std::endl;
            std::cout << "3. ping     : Provides functionality to test connectivity with host." << std::endl;
            std::cout << "4. service  : Provides managing window services through Service Control Manager (SCM)." << std::endl;
            std::cout << "5. -help    : Provides usage guide." << std::endl;
            std::cout << "6. -version : Displays application version information." << std::endl;
            std::cout << std::endl;

            std::cout << "Command Options - " << std::endl;
            std::cout << "1. wmi command options" << std::endl;
            std::cout << "    cpu" << std::endl;
            std::cout << "    logical.disk" << std::endl;
            std::cout << "    disk.drive" << std::endl;
            std::cout << "    disk.partition" << std::endl;
            std::cout << "    physical.memory" << std::endl;
            std::cout << "    computer.system" << std::endl;
            std::cout << "    operating.system" << std::endl;
            std::cout << "    os.hot.fixes" << std::endl;
            std::cout << "    network.interface" << std::endl;
            std::cout << "    bios" << std::endl;
            std::cout << "    system.users" << std::endl;
            std::cout << std::endl;

            std::cout << "2. telnet command options" << std::endl;
            std::cout << "    ip=xxx.xxx.xxx.xxx" << std::endl;
            std::cout << "    port=xxxxx" << std::endl;
            std::cout << std::endl;

            std::cout << "3. ping command option" << std::endl;
            std::cout << "    ip=xxx.xxx.xxx.xxx" << std::endl;
            std::cout << std::endl;

            std::cout << "4. service command option" << std::endl;
            std::cout << "   [Argument containing spaces must be quoted]" << std::endl;
            std::cout << "    service.query.all format=vertical/horizontal" << std::endl;
            std::cout << "    service.query name=xxxx format=vertical/horizontal" << std::endl;
            std::cout << "    service.install name=xxxx displayname=\"xxxxxxxxxx\" description=\"xxxxx xxxx xxxx\" binarypath=\"xxxxx\\xxxx\\xxx.exe\"" << std::endl;
            std::cout << "    service.uninstall name=xxxx" << std::endl;
            std::cout << "    service.start name=xxxx" << std::endl;
            std::cout << "    service.stop name=xxxx" << std::endl;
            std::cout << "    service.startup.type.change name=xxxx type=auto/manual/disable" << std::endl;
            std::cout << "    service.description name=xxxx" << std::endl;
            std::cout << std::endl;

            return 0;
        } 
        else if (strcmp(argv[i], "-version") == 0)
        {
            std::cout << "Version - 1.0.0, Published - 12 Dec 2025" << std::endl;
            return 0;
        }
        else if (strcmp(argv[i], "wmi") == 0 || 
                 strcmp(argv[i], "telnet") == 0 ||
                 strcmp(argv[i], "ping") == 0 || 
                 strcmp(argv[i], "service") == 0)
        {
            command = argv[i];
        }
        else if (strcmp(argv[i], "cpu") == 0 || 
            strcmp(argv[i], "logical.disk") == 0 || 
            strcmp(argv[i], "disk.drive") == 0 || 
            strcmp(argv[i], "disk.partition") == 0 || 
            strcmp(argv[i], "physical.memory") == 0 || 
            strcmp(argv[i], "computer.system") == 0 || 
            strcmp(argv[i], "operating.system") == 0 || 
            strcmp(argv[i], "os.hot.fixes") == 0 || 
            strcmp(argv[i], "network.interface") == 0 || 
            strcmp(argv[i], "bios") == 0 || 
            strcmp(argv[i], "system.users") == 0)
        {
            option = argv[i];
        }
        else if (strncmp(argv[i], "ip=", 3) == 0)
        {    
            option = argv[i];
            ipAddress = argv[i] + 3;
        }
        else if (strncmp(argv[i], "port=", 5) == 0)
        {
            option = argv[i];
            port = argv[i] + 5;
        }
        else if (strncmp(argv[i], "serv=", 5) == 0)
        {
            option = argv[i];
        }
        else if (strcmp(argv[i], "service.query.all") == 0 || 
                 strcmp(argv[i], "service.query") == 0 || 
                 strcmp(argv[i], "service.install") == 0 || 
                 strcmp(argv[i], "service.uninstall") == 0 || 
                 strcmp(argv[i], "service.start") == 0 || 
                 strcmp(argv[i], "service.stop") == 0 || 
                 strcmp(argv[i], "service.startup.type.change") == 0 ||
                 strcmp(argv[i], "service.description") == 0)
        {
            option = argv[i];
        }
        else if (strncmp(argv[i], "format=", 7) == 0)
        {
            serviceQueryFormat = argv[i] + 7;
        }
        else if (strncmp(argv[i], "name=", 5) == 0)
        {
            serviceName = argv[i] + 5;
        }
        else if (strncmp(argv[i], "displayname=", 12) == 0)
        {
            serviceDisplayName = argv[i] + 12;
        }
        else if (strncmp(argv[i], "description=", 12) == 0)
        {
            serviceDescription = argv[i] + 12;
        }
        else if (strncmp(argv[i], "binarypath=", 11) == 0)
        {
            serviceBinaryPath = argv[i] + 11;
        }
        else if (strncmp(argv[i], "type=", 5) == 0)
        {
            serviceStartType = argv[i] + 5;
        }

        //std::cout << "argv[" << i << "] = " << argv[i] << std::endl;
    }

    if (command == NULL) {
        std::cout << "Error : Command not found, Please provide valid Command, Use -help for available Commands" << std::endl;
        return 1;

    }
    //std::cout << "Command - " << command << std::endl;

    if (option == NULL) {
        std::cout << "Error : Option not found, Please provide valid Option, Use -help for available Options" << std::endl;
        return 1;
    }
    //std::cout << "Option - " << option << std::endl;


    if (strcmp(command, "wmi") == 0) {
        WMIQuery wmi;
        std::string wmiQuery = wmi.getWMIQueryFromOption(option);
        wmi.printWMIQueryResults(wmiQuery);
    } 
    else if (strcmp(command, "telnet") == 0) {
        if (ipAddress == NULL || port == NULL) {
            std::cout << "Error : ip and port is required for telnet command." << std::endl;
            return 1;
        }

        Telnet telnet;
        telnet.checkIPAndPortConnectivity(ipAddress, port);
    }
    else if (strcmp(command, "ping") == 0) {
        if (ipAddress == NULL) {
            std::cout << "Error : ip is required for ping command." << std::endl;
            return 1;
        }

        Ping ping;
        ping.checkIPConnectivity(ipAddress);
    }
    else if (strcmp(command, "service") == 0) {
        ServicesSCM servicesSCM;
        if (strcmp(option, "service.query.all") == 0) {
            if (serviceQueryFormat == NULL) {
                std::cout << "Error : format is required for all service query command." << std::endl;
                return 1;
            }
            servicesSCM.getAllServiceDetails(serviceQueryFormat);
        } 
        else if (strcmp(option, "service.query") == 0) {
            if (serviceName == NULL || serviceQueryFormat == NULL) {
                std::cout << "Error : name and format is required for service query command." << std::endl;
                return 1;
            }
            servicesSCM.getServiceDetails(serviceName, serviceQueryFormat);
        }
        else if (strcmp(option, "service.install") == 0) {
            if (serviceName == NULL) {
                std::cout << "Error : name is required for service install command." << std::endl;
                return 1;
            } else if (serviceDisplayName == NULL) {
                std::cout << "Error : displayname is required for service install command." << std::endl;
                return 1;
            } else if (serviceDescription == NULL) {
                std::cout << "Error : description is required for service install command." << std::endl;
                return 1;
            } else if (serviceBinaryPath == NULL) {
                std::cout << "Error : binarypath is required for service install command." << std::endl;
                return 1;
            }
            servicesSCM.serviceInstall(serviceName, serviceDisplayName, serviceDescription, serviceBinaryPath);
        }
        else if (strcmp(option, "service.uninstall") == 0) {
            if (serviceName == NULL) {
                std::cout << "Error : name is required for service uninstall command." << std::endl;
                return 1;
            }
            servicesSCM.serviceUnInstall(serviceName);
        }
        else if (strcmp(option, "service.start") == 0) {
            if (serviceName == NULL) {
                std::cout << "Error : name is required for service start command." << std::endl;
                return 1;
            }
            servicesSCM.serviceStart(serviceName);
        }
        else if (strcmp(option, "service.stop") == 0) {
            if (serviceName == NULL) {
                std::cout << "Error : name is required for service stop command." << std::endl;
                return 1;
            }
            servicesSCM.serviceStop(serviceName);
        }
        else if (strcmp(option, "service.startup.type.change") == 0) {
            if (serviceName == NULL || serviceStartType == NULL) {
                std::cout << "Error : name and type is required for changing service startup type." << std::endl;
                return 1;
            } 
            servicesSCM.serviceStartupTypeChange(serviceName, serviceStartType);
        }
        else if (strcmp(option, "service.description") == 0) {
            if (serviceName == NULL) {
                std::cout << "Error : name is required for getting service description." << std::endl;
                return 1;
            }
            servicesSCM.getServiceDescription(serviceName);
        }

    }

    return 0;
}








