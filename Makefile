NAME=qch_vm
V_MAJOR=1
V_MINOR=0
V_MICRO=0
LIB_NAME=lib${NAME}.so
LIB_PATH=out/${LIB_NAME}
LIB_VER=${LIB_PATH}.${V_MAJOR}.${V_MINOR}.${V_MICRO}

CXX=g++
LD_FLAGS=-Wl,-soname,${LIB_NAME} -shared
CXX_FLAGS=-std=c++17 -fPIC -I./headers

SOURCES=$(wildcard src/*.cpp) $(wildcard src/*/*.cpp)
OBJECTS=$(patsubst src/%,build/%,${SOURCES:.cpp=.o})
DIRS=$(filter-out build/,$(sort $(dir ${OBJECTS}))) build/include

ifdef DEBUG
CXX_FLAGS += -g -DDEBUG
endif
ifndef DEBUG
CXX_FLAGS += -O2
endif

all: dirs ${LIB_VER}

${LIB_VER}: ${OBJECTS}
	${CXX} $^  ${LD_FLAGS} -o $@

build/%.o: src/%.cpp
	${CXX} $< ${CXX_FLAGS} -c -o $@

.PHONY: dirs
dirs:
	mkdir -p ${DIRS}
	mkdir -p out/

.PHONY: clean
clean:
	-rm -r build/
	-rm -r out/

.PHONY: install
install:
	cp ${LIB_VER} /usr/local/lib
	mkdir -p /usr/local/include/${NAME}/
	cp -r headers/* /usr/local/include/${NAME}/

.PHONY: uninstall
uninstall:
	rm /usr/local/lib/${LIB_NAME}*
	rm -r /usr/local/include/${NAME}
