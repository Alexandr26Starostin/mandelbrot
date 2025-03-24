CC=g++
FLAGS=-Iinclude -Iinclude -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Weffc++ -Wmain -Wextra -Wall -g -pipe -fexceptions -Wcast-qual -Wconversion -Wctor-dtor-privacy -Wempty-body -Wformat-security -Wformat=2 -Wignored-qualifiers -Wlogical-op -Wmissing-field-initializers -Wnon-virtual-dtor -Woverloaded-virtual -Wpointer-arith -Wsign-promo -Wstack-usage=8192 -Wstrict-aliasing -Wstrict-null-sentinel -Wtype-limits -Wwrite-strings -D_DEBUG -D_EJUDGE_CLIENT_SIDE -D_EJC
FILES=main.o draw_mandelbrod.o
TARGET=mandelbrod
SFML_TARGETS=sfml-app -lsfml-graphics -lsfml-window -lsfml-network -lsfml-system #-lsfml-audio

# pathsub
# addprefix
# wildcard
#g++ main.o -o sfml-app -lsfml-graphics -lsfml-window -lsfml-audio -lsfml-network -lsfml-system

PATCHED_FILES = $(addprefix build/, $(FILES))

all: prepare $(TARGET)

prepare:
	mkdir -p build

$(TARGET): $(FILES)
	@echo "Linling..."
	@$(CC) $(FLAGS) $(PATCHED_FILES) -o $(SFML_TARGETS) -o $(TARGET)

main.o: src/main.cpp
	@$(CC) -c $(FLAGS) src/main.cpp -o build/main.o

draw_mandelbrod.o: src/draw_mandelbrod.cpp
	@$(CC) -c $(FLAGS) src/draw_mandelbrod.cpp -o build/draw_mandelbrod.o

clean:
	rm -rf $(TARGET)
	cd build && rm -rf *.o