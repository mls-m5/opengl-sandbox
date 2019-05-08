flags= $< shaderprogram.cpp -std=c++11 -g -o $@ -lSDL2 -lGL  -fmax-errors=5 -O0 -W -Wall -Wno-unused-parameter -Wno-sign-compare -Wno-shadow  -Wno-narrowing -Imatengine/
testcpp= ${wildcard test*.cpp}
targets= ${testcpp:.cpp=}


all: ${targets}
	echo ${targets}

test%: test%.cpp shaderprogram.h shaderprogram.cpp
	c++ ${flags}


clean:
	rm -f ${targets}