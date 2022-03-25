#!/bin/bash

#g++ -O2 -s -Wall -Wextra -Wpedantic -std=c++17 -o test MazeGen_New_1.cpp city.cc
#g++ -Og -g -Wall -Wextra -Wpedantic -std=c++17 -o test MazeGen_New_1.cpp city.cc

#x86_64-w64-mingw32-g++.exe -O2 -s -Wall -Wextra -Wpedantic -std=c++17 -D_MSC_VER -c -o city_w.o city.cc
#x86_64-w64-mingw32-g++.exe -O2 -s -Wall -Wextra -Wpedantic -std=c++17 -o test MazeGen_New_1.cpp city_w.o

x86_64-w64-mingw32-g++.exe -O3 -s -Wall -Wextra -Wpedantic -std=c++17 -D_MSC_VER -c -o city_w.o city.cc
x86_64-w64-mingw32-g++.exe -O2 -s -Wall -Wextra -Wpedantic -std=c++17 -o Giant main.cpp MazeGen_New_1.cpp city_w.o -luser32 -lgdi32 -lgdiplus -lopengl32 -lSHlwapi -ldwmapi -lstdc++fs -lwinmm -Wl,-subsystem,windows
#x86_64-w64-mingw32-g++.exe -O3 -s -Wall -Wextra -Wpedantic -std=c++17 -o Giant main.cpp MazeGen_New_1.cpp city_w.o -luser32 -lgdi32 -lgdiplus -lopengl32 -lSHlwapi -ldwmapi -lstdc++fs -lwinmm

#x86_64-w64-mingw32-g++.exe -Og -g -Wall -Wextra -Wpedantic -std=c++17 -D_MSC_VER -c -o city_w.o city.cc
#x86_64-w64-mingw32-g++.exe -Og -g -Wall -Wextra -Wpedantic -std=c++17 -o Giant main.cpp MazeGen_New_1.cpp city_w.o -luser32 -lgdi32 -lgdiplus -lopengl32 -lSHlwapi -ldwmapi -lstdc++fs -lwinmm -Wl,-subsystem,windows
#x86_64-w64-mingw32-g++.exe -Og -g -Wall -Wextra -Wpedantic -std=c++17 -o Giant main.cpp MazeGen_New_1.cpp city_w.o -luser32 -lgdi32 -lgdiplus -lopengl32 -lSHlwapi -ldwmapi -lstdc++fs -lwinmm