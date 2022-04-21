CXXFLAGS=  -fPIC -O3 -Wpedantic -Wall

ifeq ($(MAKECMDGOALS),debug)
CXXFLAGS+= -O0 -g -lSegFault -rdynamic -DDEBUG
endif


DataModelInclude =
DataModelLib =

MyToolsInclude =
MyToolsLib =


debug: all

all: lib/libMyTools.so lib/libToolChain.so lib/libStore.so include/Tool.h lib/libDataModel.so lib/libLogging.so main

main: src/main.cpp lib/libStore.so lib/libLogging.so lib/libToolChain.so | lib/libMyTools.so  lib/libDataModel.so 
	@echo -e "\e[38;5;214m\n*************** Making " $@ "****************\e[0m"
	g++ $(CXXFLAGS) src/main.cpp -o main -I include -L lib -lStore -lMyTools -lToolChain -lDataModel -lLogging  -lpthread $(DataModelInclude) $(MyToolsInclude) $(MyToolsLib) $(DataModelLib) 


lib/libStore.so: src/Store/*

	@echo -e "\e[38;5;214m\n*************** Making " $@ "****************\e[0m"
	cp src/Store/*.h include/
	g++ $(CXXFLAGS) -shared -I include src/Store/*.cpp -o lib/libStore.so


include/Tool.h: src/Tool/Tool.h

	@echo -e "\e[38;5;214m\n*************** Making " $@ "****************\e[0m"
	cp src/Tool/Tool.h include/


lib/libToolChain.so: src/ToolChain/* lib/libStore.so include/Tool.h lib/libLogging.so |  lib/libDataModel.so lib/libMyTools.so 

	@echo -e "\e[38;5;214m\n*************** Making " $@ "****************\e[0m"
	cp src/ToolChain/ToolChain.h include/
	g++ $(CXXFLAGS) -shared src/ToolChain/ToolChain.cpp -I include -lpthread -L lib -lStore -lDataModel -lMyTools -lLogging -o lib/libToolChain.so $(DataModelInclude) $(MyToolsInclude) $(MyToolsLib) $(DataModelLib)


clean: 

	@echo -e "\e[38;5;201m\n*************** Cleaning up ****************\e[0m"
	rm -f include/*.h
	rm -f lib/*.so
	rm -f main
	rm -f UserTools/*/*.o
	rm -f DataModel/*.o

lib/libDataModel.so: DataModel/* lib/libLogging.so lib/libStore.so  $(patsubst DataModel/%.cpp, DataModel/%.o, $(wildcard DataModel/*.cpp))

	@echo -e "\e[38;5;214m\n*************** Making " $@ "****************\e[0m"
	g++ $(CXXFLAGS) -shared DataModel/*.o -I include -L lib -lStore -lLogging -o lib/libDataModel.so $(DataModelInclude) $(DataModelLib)


lib/libMyTools.so: UserTools/*/* UserTools/* lib/libStore.so include/Tool.h lib/libLogging.so UserTools/Factory/Factory.o | lib/libDataModel.so 

	@echo -e "\e[38;5;214m\n*************** Making " $@ "****************\e[0m"
	g++ $(CXXFLAGS) -shared UserTools/*/*.o -I include -L lib -lStore -lDataModel -lLogging -o lib/libMyTools.so $(MyToolsInclude) $(DataModelInclude) $(MyToolsLib) $(DataModelLib)


lib/libLogging.so: src/Logging/*  lib/libStore.so 
	@echo -e "\e[38;5;214m\n*************** Making " $@ "****************\e[0m"
	cp src/Logging/Logging.h include/
	g++ $(CXXFLAGS) -shared -I include src/Logging/Logging.cpp -o lib/libLogging.so -L lib/ -lStore


UserTools/Factory/Factory.o: UserTools/Factory/Factory.cpp lib/libStore.so include/Tool.h lib/libLogging.so lib/libDataModel.so $(patsubst UserTools/%.cpp, UserTools/%.o, $(wildcard UserTools/*/*.cpp)) | include/Tool.h
	@echo -e "\e[38;5;214m\n*************** Making " $@ "****************\e[0m"
	cp UserTools/Factory/Factory.h include
	cp UserTools/Unity.h include
	-g++ $(CXXFLAGS) -c -o $@ $< -I include -L lib -lStore -lDataModel -lLogging $(MyToolsInclude) $(MyToolsLib) $(DataModelInclude) $(DataModelLib)


UserTools/%.o: UserTools/%.cpp lib/libStore.so include/Tool.h lib/libLogging.so lib/libDataModel.so | include/Tool.h
	@echo -e "\e[38;5;214m\n*************** Making " $@ "****************\e[0m"
	cp $(shell dirname $<)/*.h include
	-g++ $(CXXFLAGS) -c -o $@ $< -I include -L lib -lStore -lDataModel -lLogging $(MyToolsInclude) $(MyToolsLib) $(DataModelInclude) $(DataModelLib)


target: remove $(patsubst %.cpp, %.o, $(wildcard UserTools/$(TOOL)/*.cpp))

remove:
	@echo -e "removing"
	-rm UserTools/$(TOOL)/*.o
	-rm include/$(TOOL).h


DataModel/%.o: DataModel/%.cpp lib/libLogging.so lib/libStore.so  
	@echo -e "\e[38;5;214m\n*************** Making " $@ "****************\e[0m"
	cp $(shell dirname $<)/*.h include
	-g++ $(CXXFLAGS) -c -o $@ $< -I include -L lib -lStore -lLogging  $(DataModelInclude) $(DataModelLib)

Docs:
	doxygen Doxyfile
