compileArguments=(-std=c++11 -Wall -DPLATFORM_MACOS=1)

if [ "$1" == "-d" ]; then
    # Debug build
    compileArguments+=(-g)
elif [ "$1" == "-p" ]; then
    # Debug build
    compileArguments+=(-g -O2 -shared-libgcc -fno-omit-frame-pointer)
else
    compileArguments+=(-O2)
fi

g++ ${compileArguments[@]} -o bin/raytracer main.cpp -lpthread && bin/raytracer && open render.bmp
