

REM Register the application to start the application automatically when the user logins
reg delete "HKLM\Software\Microsoft\Windows\CurrentVersion\Run" /v "WindowsActivityLogger" /f
