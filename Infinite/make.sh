#!/bin/bash

x86_64-w64-mingw32-g++.exe -O3 -s -Wall -Wextra -Wpedantic -std=c++17 -D_MSC_VER -c -o city_w.o external/city.cc
##x86_64-w64-mingw32-g++.exe -O2 -s -Wall -Wextra -Wpedantic -std=c++17 -o test test.cpp create.cpp MakeBMP.cpp MetaZone.cpp QuatroStack.cpp Zone.cpp city_w.o
##x86_64-w64-mingw32-g++.exe -O2 -s -Wall -Wextra -Wpedantic -std=c++17 -o test test2.cpp create.cpp MakeBMP.cpp MetaZone.cpp QuatroStack.cpp Zone.cpp city_w.o
x86_64-w64-mingw32-g++.exe -O2 -s -Wall -Wextra -Wpedantic -std=c++17 -o test test3.cpp create.cpp MakeBMP.cpp MetaZone.cpp QuatroStack.cpp solve.cpp Zone.cpp city_w.o
#x86_64-w64-mingw32-g++.exe -O2 -s -Wall -Wextra -Wpedantic -std=c++17 -o Giant main.cpp MazeGen_New_1.cpp city_w.o -luser32 -lgdi32 -lgdiplus -lopengl32 -lSHlwapi -ldwmapi -lstdc++fs -lwinmm -Wl,-subsystem,windows
#x86_64-w64-mingw32-g++.exe -O3 -s -Wall -Wextra -Wpedantic -std=c++17 -o Giant main.cpp MazeGen_New_1.cpp city_w.o -luser32 -lgdi32 -lgdiplus -lopengl32 -lSHlwapi -ldwmapi -lstdc++fs -lwinmm

#x86_64-w64-mingw32-g++.exe -Og -g -Wall -Wextra -Wpedantic -std=c++17 -D_MSC_VER -c -o city_w.o external/city.cc
#x86_64-w64-mingw32-g++.exe -Og -g -Wall -Wextra -Wpedantic -std=c++17 -o Giant main.cpp MazeGen_New_1.cpp city_w.o -luser32 -lgdi32 -lgdiplus -lopengl32 -lSHlwapi -ldwmapi -lstdc++fs -lwinmm -Wl,-subsystem,windows
#x86_64-w64-mingw32-g++.exe -Og -g -Wall -Wextra -Wpedantic -std=c++17 -o Giant main.cpp MazeGen_New_1.cpp city_w.o -luser32 -lgdi32 -lgdiplus -lopengl32 -lSHlwapi -ldwmapi -lstdc++fs -lwinmm