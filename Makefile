flags= $< -std=c++11 -g -o $@ -lSDL2 -lGL  -fmax-errors=5
flags+= matgui/src/shaderprogram.cpp 
flags+= -O0 -W -Wall -Wno-unused-parameter -Wno-sign-compare -Wno-shadow  -Wno-narrowing
flags+= -Imatgui/include/matgui  -Imatengine/
testcpp= ${wildcard test*.cpp}
targets= ${testcpp:.cpp=}


all: ${targets}
	@echo built ${targets}

test%: test%.cpp #matgui/include/shaderprogram.h matgui/src/shaderprogram.cpp matgui/include/matgui/matgl.h matsdl.h
	c++ ${flags}


clean:
	rm -f ${targets}