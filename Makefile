CXX = g++
CXXFLAGS = -Wall

SRCS = server/server.c server/crypt.c server/frames.c server/http_p.cpp server/hmap.cpp
TARGET = dist/server

all: npm_build $(TARGET)  

$(TARGET): $(SRCS)
	mkdir -p dist
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

npm_build:
	npm run build

run: $(TARGET) npm_build
	./$(TARGET)

clean:
	rm -rf dist/
