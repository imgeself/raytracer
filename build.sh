compileArguments=(-std=c++11 -Wall)

if [ "$1" == "-d" ]; then
    # Debug build
    compileArguments+=(-g)
else
    compileArguments+=(-O2)
fi

clang++ ${compileArguments[@]} -o bin/raytracer main.cpp image.cpp -lpthread && bin/raytracer && open render.bmp
