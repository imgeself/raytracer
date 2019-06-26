compileArguments=(-std=c++11 -Wall -march=native)

if [ "$1" == "-d" ]; then
    # Debug build
    compileArguments+=(-g -DSINGLE_THREAD)
elif [ "$1" == "-p" ]; then
    # Debug build
    compileArguments+=(-g -O2 -shared-libgcc -fno-omit-frame-pointer)
else
    compileArguments+=(-O2)
fi

if [[ "$OSTYPE" == "darwin"* ]]; then 
    compileArguments+=(-DPLATFORM_MACOS=1)
elif [[ "$OSTYPE" == "linux-gnu" ]]; then
    compileArguments+=(-DPLATFORM_LINUX=1)
fi

mkdir bin
g++ ${compileArguments[@]} -o bin/raytracer main.cpp -lpthread && bin/raytracer && open render.bmp
