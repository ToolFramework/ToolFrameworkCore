SOURCEDIR:=`pwd`

CXXFLAGS= -fPIC -O3 -Wpedantic -Wall -std=c++11 -Wno-comment -Wno-unused -Wextra -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept  -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wswitch-default -Wundef #-Werror -Wold-style-cast 


ifeq ($(MAKECMDGOALS),debug)
CXXFLAGS+= -O0 -g -lSegFault -rdynamic -DDEBUG
endif

ifeq ($(MAKECMDGOALS),no_colour)
CXXFLAGS+= -DNO_COLOUR
endif

TempDataModelInclude =
TempDataModelLib =

TempToolsInclude =
TempToolsLib =


Includes=-I $(SOURCEDIR)/include/ -I $(SOURCEDIR)/tempinclude/
Libs=-L $(SOURCEDIR)/lib/  -lToolChain  -lTempDataModel -lTempTools -lDataModelBase -lLogging -lStore -lpthread
LIBRARIES=lib/libStore.so lib/libLogging.so lib/libToolChain.so lib/libDataModelBase.so lib/libTempDataModel.so lib/libTempTools.so
HEADERS:=$(patsubst %.h, include/%.h, $(filter %.h, $(subst /, ,$(wildcard src/*/*.h) )))
TempDataModelHEADERS:=$(patsubst %.h, tempinclude/%.h, $(filter %.h, $(subst /, ,$(wildcard DataModel/*.h))))
TempMyToolHEADERS:=$(patsubst %.h, tempinclude/%.h, $(filter %.h, $(subst /, ,$(wildcard UserTools/*/*.h) $(wildcard UserTools/*.h))))
SOURCEFILES:=$(patsubst %.cpp, %.o, $(wildcard */*.cpp) $(wildcard */*/*.cpp))

#.SECONDARY: $(%.o)


all: $(HEADERS) $(TempDataModelHEADERS) $(TempMyToolHEADERS) $(SOURCEFILES) $(LIBRARIES) main


no_colour: all
debug: all

main: src/main.o $(LIBRARIES) $(HEADERS) $(TempDataModelHEADERS) $(TempMyToolHEADERS) | $(SOURCEFILES)
	@echo -e "\e[38;5;11m\n*************** Making " $@ " ****************\e[0m"
	g++  $(CXXFLAGS) $< -o $@ $(Includes) $(Libs) $(TempDataModelInclude) $(TempDataModellib) $(TempToolsInclude) $(TempToolslib) 

include/%.h:
	@echo -e "\e[38;5;87m\n*************** sym linking headers ****************\e[0m"
	ln -s  $(SOURCEDIR)/$(filter %$(strip $(patsubst include/%.h, /%.h, $@)), $(wildcard src/*/*.h) $(wildcard UserTools/*/*.h)) $@

tempinclude/%.h:
	@echo -e "\e[38;5;87m\n*************** sym linking headers ****************\e[0m"
	ln -s  $(SOURCEDIR)/$(filter %$(strip $(patsubst tempinclude/%.h, /%.h, $@)), $(wildcard DataModel/*.h) $(wildcard UserTools/*/*.h) $(wildcard UserTools/*.h)) $@

src/%.o :  src/%.cpp $(HEADERS)  
	@echo -e "\e[38;5;214m\n*************** Making " $@ "****************\e[0m"
	g++ $(CXXFLAGS) -c $< -o $@ $(Includes)

UnitTests/%.o : UnitTests/%.cpp $(HEADERS) 
	@echo -e "\e[38;5;214m\n*************** Making " $@ "****************\e[0m"
	g++ $(CXXFLAGS) -c $< -o $@ $(Includes)

UserTools/Factory/Factory.o :  UserTools/Factory/Factory.cpp $(HEADERS) $(TempDataModelHEADERS)
	@echo -e "\e[38;5;214m\n*************** Making " $@ "****************\e[0m"
	g++ $(CXXFLAGS) -c $< -o $@ $(Includes) $(TempDataModelInclude) $(TempToolsInclude)

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

lib/libDataModelBase.so: $(patsubst %.cpp, %.o , $(wildcard src/DataModelBase/*.cpp)) | $(HEADERS)
	@echo -e "\e[38;5;201m\n*************** Making " $@ "****************\e[0m"
	g++ $(CXXFLAGS) --shared $^ -o $@ $(Includes) 

lib/libToolChain.so: $(patsubst %.cpp, %.o , $(wildcard src/ToolChain/*.cpp)) | $(HEADERS)
	@echo -e "\e[38;5;201m\n*************** Making " $@ "****************\e[0m"
	g++ $(CXXFLAGS) --shared $^ -o $@ $(Includes)

lib/libTempDataModel.so: $(patsubst %.cpp, %.o , $(wildcard DataModel/*.cpp)) | $(HEADERS) $(TempDataModelHEADERS)
	@echo -e "\e[38;5;201m\n*************** Making " $@ "****************\e[0m"
	g++ $(CXXFLAGS) --shared $^ -o $@ $(Includes) $(TempDataModelInclude)

lib/libTempTools.so: $(patsubst %.cpp, %.o , $(wildcard UserTools/*/*.cpp)) | $(HEADERS) $(TempDataModelHEADERS) $(TempMyToolHEADERS)
	@echo -e "\e[38;5;201m\n*************** Making " $@ "****************\e[0m"
	g++ $(CXXFLAGS) --shared $^ -o $@ $(Includes) $(TempDataModelInclude) $(TempToolsInclude)

clean:
	@echo -e "\e[38;5;201m\n*************** Cleaning up ****************\e[0m"
	rm -f */*/*.o
	rm -f */*.o
	rm -f include/*.h
	rm -f tempinclude/*.h
	rm -f lib/*.so

Docs:
	doxygen Doxyfile


test: $(patsubst %.cpp, %.o, $(wildcard */*.cpp) $(wildcard */*/*.cpp)) 
	echo 
	echo  $(patsubst %.cpp, %.o, $(wildcard */*.cpp) $(wildcard */*/*.cpp)) 
#	echo $(patsubst %, lib/Lib%.so, $(filter-out %.o %.cpp src, $(subst /, , $(wildcard src/*))))

test2:
	echo $(patsubst %.h, include/%.h, $(filter %.h, $(subst /, ,$(wildcard src/*/*.h) )))
	echo $(patsubst %.h, include/%.h, $(filter %.h, $(subst /, ,$(patsubst src/%.h, include/%.h, $(wildcard src/*/*.h) ))))
