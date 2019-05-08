flags= $< shaderprogram.cpp -std=c++11 -g -o $@ -lSDL2 -lGL  -fmax-errors=5 -O0 -W -Wall -Wno-unused-parameter -Wno-sign-compare -Wno-shadow  -Wno-narrowing -Imatengine/
testcpp= ${wildcard test*.cpp}
targets= ${testcpp:.cpp=}


all: ${targets}
	@echo built ${targets}

test%: test%.cpp shaderprogram.h shaderprogram.cpp matgl.h matsdl.h
	c++ ${flags}


clean:
	rm -f ${targets}