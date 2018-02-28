all: assgn2

assgn2: assgn2.cpp
	g++ -g -o assgn2 assgn2.cpp -lglfw -lGLEW -lGL -ldl

clean:
	rm assgn2
