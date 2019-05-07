

flags= $< shaderprogram.cpp -std=c++11 -g -o $@ -lSDL2 -lGL  -fmax-errors=5 -O0 -W -Wall -Wno-unused-parameter -Wno-sign-compare -Wno-shadow

targets= test1 test2 test3 test4 test5 test6-intel  test7-framebuffer test8-multitextures


all: ${targets}

%: %.cpp shaderprogram.h shaderprogram.cpp
	./$<


test8-multitextures: test8-multitextures.cpp
	c++ ${flags}
	

clean:
	rm -f ${targets}