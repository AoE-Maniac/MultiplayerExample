copy build\Debug\MPShmup.exe Deployment\Test.exe
cd Deployment
start Test.exe 27734
timeout /t 3 /nobreak
start Test.exe [27403 localhost 27734]
start Test.exe [27404 localhost 27734]