@echo off
REM SSAAS — Build script for Windows / MinGW
REM Compiles the full C++14 backend into build\ssaas_server.exe

setlocal
set "GPP=C:\MinGW\bin\g++.exe"
if not exist "%GPP%" set "GPP=g++"

if not exist build mkdir build

echo [1/2] Compiling SSAAS C++ backend ...
"%GPP%" -std=c++14 -Wall -Wextra -O2 ^
    -Iinclude -Iinclude\third_party ^
    main.cpp ^
    src\core\Date.cpp src\core\Person.cpp src\core\Subject.cpp ^
    src\core\AttendanceRecord.cpp src\core\MarksRecord.cpp ^
    src\core\Exam.cpp src\core\Student.cpp ^
    src\analytics\HealthScoreAnalyzer.cpp src\analytics\BunkPredictor.cpp ^
    src\analytics\AttendanceRiskAnalyzer.cpp src\analytics\PerformanceAnalyzer.cpp ^
    src\analytics\PressureAnalyzer.cpp src\analytics\GradePredictor.cpp ^
    src\analytics\BayesianRiskScorer.cpp src\analytics\BurnoutDetector.cpp ^
    src\analytics\ExamCountdownAnalyzer.cpp src\analytics\StudyPlanAnalyzer.cpp ^
    src\alerts\IAlert.cpp src\alerts\AttendanceAlert.cpp ^
    src\alerts\PerformanceAlert.cpp src\alerts\ExamAlert.cpp ^
    src\alerts\BurnoutAlert.cpp src\alerts\AlertFactory.cpp ^
    src\alerts\AlertSystem.cpp ^
    src\storage\DataStore.cpp ^
    src\server\HttpRequest.cpp src\server\HttpResponse.cpp ^
    src\server\Router.cpp src\server\HttpServer.cpp ^
    src\server\Controllers.cpp ^
    -o build\ssaas_server.exe ^
    -lws2_32 -lwsock32

if errorlevel 1 (
    echo [X] Build failed.
    exit /b 1
)

echo [2/2] Build OK -^> build\ssaas_server.exe
echo.
echo Run with:
echo   cd backend\build
echo   ssaas_server.exe --data ..\data\sample_data.json --static ..\..\frontend
endlocal
