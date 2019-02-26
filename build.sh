compileArguments="-std=c++11 -Wall"

if [ "$1" == "-d" ]; then
    # Debug build
    compileArguments+=" -g"
else
    compileArguments+=" -O2"
fi

g++ $compileArguments -o bin/raytracer main.cpp image.cpp && bin/raytracer && open render.bmp
