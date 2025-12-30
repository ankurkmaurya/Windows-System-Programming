#pragma once

class ServicesSCM
{

public:
    void getAllServiceDetails(char* format);

    void getServiceDetails(char* serviceName, char* format);

    void getServiceDescription(char* serviceName);

    void serviceInstall(char* serviceName,
        char* serviceDisplayName,
        char* serviceDescription,
        char* serviceBinaryPath);

    void serviceUnInstall(char* serviceName);

    bool serviceStart(char* serviceName);

    bool serviceStop(char* serviceName);

    bool serviceStartupTypeChange(char* serviceName, char* serviceStartType);
};
