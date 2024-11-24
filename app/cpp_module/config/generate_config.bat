@echo off
setlocal enabledelayedexpansion

:: Load environment variables
for /f "tokens=*" %%a in ('type ..\..\..\..\\.env') do (
    set %%a
)

:: Read template
set "template="
for /f "delims=" %%a in (config.template.json) do (
    set "line=%%a"
    set "line=!line:3000=%APP_PORT%!"
    set "line=!line:4=%NUM_THREADS%!"
    set "line=!line:localhost=%DB_HOST%!"
    set "line=!line:5432=%DB_PORT%!"
    set "line=!line:fileservice=%DB_NAME%!"
    set "line=!line:"user": "postgres"="user": "%DB_USER%"!"
    set "line=!line:"passwd": "postgres"="passwd": "%DB_PASSWORD%"!"
    set "line=!line:your_secret_key_here=%JWT_SECRET%!"
    echo !line!
) > config.json
