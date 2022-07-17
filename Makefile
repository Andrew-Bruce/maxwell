compiler = g++
flags := -Wall -lglfw -lGLEW -lGL

srcs := main.cpp
srcs += draw.cpp
srcs += game.cpp

foo:	$(srcs)
	$(compiler) $(flags) $< -o $(@)

clean:
	rm *~
