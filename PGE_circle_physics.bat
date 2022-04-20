:: C++ compiler settings
g++ -o PGE_circle_physics.exe PGE_circle_physics.cpp -static-libstdc++ -lpthread -lsetupapi -lwinmm -luser32 -lgdi32 -lgdiplus -static -lopengl32 -lShlwapi -ldwmapi -lstdc++fs -std=c++20

:: this runs the program !! make sure you're in the right directory !!
start PGE_circle_physics
