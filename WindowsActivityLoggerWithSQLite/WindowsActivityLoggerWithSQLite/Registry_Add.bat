

REM Register the application to start the application automatically when the user logins
reg add "HKLM\Software\Microsoft\Windows\CurrentVersion\Run" /v "WindowsActivityLogger" /t REG_SZ /d "\"D:\Karyasthal\git-repos\Windows-System-Programming\WindowsActivityLoggerWithSQLite\x64\Release\WindowsActivityLoggerWithSQLite.exe\" -logtofile -disableconsole" /f







