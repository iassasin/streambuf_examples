CC = g++
CPPFLAGS = -std=c++11 -Wall -O2
LDLIBS = 

SOURCES = $(wildcard *.cpp)

APP_NAME = num_filter
APP = $(APP_NAME)

all: $(APP)
	strip $(APP)
	
clean:
	rm -f $(APP) $(SOURCES:%.cpp=%.o)

$(APP): $(SOURCES:%.cpp=%.o)
	$(LINK.o) $^ $(LDLIBS) -o $(APP)

