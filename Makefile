SOURCEDIR=/host/Dependencies/ToolFrameworkCore

CXXFLAGS=  -fPIC -O3 -Wpedantic -Wall -std=c++11 -Wno-comment -Wno-unused -Wextra -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept  -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wswitch-default -Wundef #-Werror -Wold-style-cast 


ifeq ($(MAKECMDGOALS),debug)
CXXFLAGS+= -O0 -g -lSegFault -rdynamic -DDEBUG
endif

TempDataModelInclude =
TempDataModelLib =

TempToolsInclude =
TempToolsLib =


Includes=-I $(SOURCEDIR)/include/
Libs=-L $(SOURCEDIR)/lib/ -lStore -lLogging -lToolchain -lTempDataModelBase -lTempDataModel -lTempTools -lpthread
LIBRARIES=lib/libStore.so lib/libLogging.so lib/libToolChain.so lib/libTempDataModelbase.so lib/libTempDataModel.so lib/libTempTools.so
HEADERS:=$(patsubst %.h, include/%.h, $(filter %.h, $(subst /, ,$(patsubst src/%.h, include/%.h, $(wildcard src/*/*.h) ))))
TempDataModelHEADERS:=$(patsubst %.h, include/%.h, $(filter %.h, $(subst /, ,$(patsubst src/%.h, include/%.h, $(wildcard DataModel/*.h)))))
MyToolHEADERS:=$(patsubst %.h, include/%.h, $(filter %.h, $(subst /, ,$(patsubst src/%.h, include/%.h, $(wildcard UserTools/*/*.h) $(wildcard UserTools/*.h)))))
SOURCEFILES:=$(patsubst %.cpp, %.o, $(wildcard */*.cpp) $(wildcard */*/*.cpp))

#.SECONDARY: $(%.o)


all: $(HEADERS) $(TempDataModelHEADERS) $(MyToolHEADERS) $(SOURCEFILES) $(LIBRARIES) main

debug: all

main: src/main.o $(LIBRARIES) $(HEADERS) $(TempDataModelHEADERS) $(MyToolHEADERS) | $(SOURCEFILES)
	@echo -e "\e[38;5;11m\n*************** Making " $@ " ****************\e[0m"
	g++  $(CXXFLAGS) $< -o $@ $(Includes) $(Libs) $(TempDataModelInclude) $(TempDataModellib) $(TempToolsInclude) $(TempToolslib) 

include/%.h:
	@echo -e "\e[38;5;87m\n*************** sym linking headers ****************\e[0m"
	ln -s  `pwd`/$(filter %$(strip $(patsubst include/%.h, /%.h, $@)), $(wildcard src/*/*.h) $(wildcard DataModel/*.h) $(wildcard UserTools/*/*.h) $(wildcard UserTools/*.h)) $@

src/%.o :  src/%.cpp $(HEADERS)  
	@echo -e "\e[38;5;214m\n*************** Making " $@ "****************\e[0m"
	g++ $(CXXFLAGS) -c $< -o $@ $(Includes)

UnitTests/%.o : UnitTests/%.cpp $(HEADERS) 
	@echo -e "\e[38;5;214m\n*************** Making " $@ "****************\e[0m"
	g++ $(CXXFLAGS) -c $< -o $@ $(Includes)

UserTools/%.o :  UserTools/%.cpp $(HEADERS) $(TempDataModelHEADERS) UserTools/%.h
	@echo -e "\e[38;5;214m\n*************** Making " $@ "****************\e[0m"
	g++ $(CXXFLAGS) -c $< -o $@ $(Includes) $(TempDataModelInclude) $(TempToolsInclude)

DataModel/%.o : DataModel/%.cpp DataModel/%.h $(HEADERS) $(TempDataModelHEADERS)
	@echo -e "\e[38;5;214m\n*************** Making " $@ "****************\e[0m"
	g++ $(CXXFLAGS) -c $< -o $@ $(Includes) $(TempDataModelInclude)

lib/libStore.so: $(patsubst %.cpp, %.o , $(wildcard src/Store/*.cpp)) | $(HEADERS) 
	@echo -e "\e[38;5;201m\n*************** Making " $@ "****************\e[0m"
	g++ $(CXXFLAGS) --shared $^ -o $@ $(Includes)

lib/libLogging.so: $(patsubst %.cpp, %.o , $(wildcard src/Logging/*.cpp)) | $(HEADERS) 
	@echo -e "\e[38;5;201m\n*************** Making " $@ "****************\e[0m"
	g++ $(CXXFLAGS) --shared $^ -o $@ $(Includes)

lib/libTempDataModelbase.so: $(patsubst %.cpp, %.o , $(wildcard src/DataModelBase/*.cpp)) | $(HEADERS)
	@echo -e "\e[38;5;201m\n*************** Making " $@ "****************\e[0m"
	g++ $(CXXFLAGS) --shared $^ -o $@ $(Includes) 

lib/libToolChain.so: $(patsubst %.cpp, %.o , $(wildcard src/ToolChain/*.cpp)) | $(HEADERS)
	@echo -e "\e[38;5;201m\n*************** Making " $@ "****************\e[0m"
	g++ $(CXXFLAGS) --shared $^ -o $@ $(Includes)

lib/libTempDataModel.so: $(patsubst %.cpp, %.o , $(wildcard DataModel/*.cpp)) | $(HEADERS) $(TempDataModelInclude)
	@echo -e "\e[38;5;201m\n*************** Making " $@ "****************\e[0m"
	g++ $(CXXFLAGS) --shared $^ -o $@ $(Includes) $(TempDataModelInclude)

lib/libTempTools.so: $(patsubst %.cpp, %.o , $(wildcard UserTools/*/*.cpp)) | $(HEADERS) $(TempDataModelInclude) $(TempToolsInclude)
	@echo -e "\e[38;5;201m\n*************** Making " $@ "****************\e[0m"
	g++ $(CXXFLAGS) --shared $^ -o $@ $(Includes) $(TempDataModelInclude) $(TempToolsInclude)

clean:
	@echo -e "\e[38;5;201m\n*************** Cleaning up ****************\e[0m"
	rm -f */*/*.o
	rm -f */*.o
	rm -f include/*.h
	rm -f lib/*.so

Docs:
	doxygen Doxyfile


test: $(patsubst %.cpp, %.o, $(wildcard */*.cpp) $(wildcard */*/*.cpp)) 
	echo 
	echo  $(patsubst %.cpp, %.o, $(wildcard */*.cpp) $(wildcard */*/*.cpp)) 
#	echo $(patsubst %, lib/Lib%.so, $(filter-out %.o %.cpp src, $(subst /, , $(wildcard src/*))))
