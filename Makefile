SOURCEDIR := ./src
INCLUDEDIR := ./include/
LIBDIR := ./lib 
DIR := .
EXE := ./bin/server.exe
CONFIG_PATH:=./conf
CC	:= g++
LIBS:= json
OCPP:= $(patsubst $(SOURCEDIR)/%.cpp, $(SOURCEDIR)/%.o, $(wildcard $(SOURCEDIR)/*.cpp))
OBJS:= $(OCPP)
RM	:= rm -rf
CXXFLAGS:= -Wall -g -std=c++11
start: $(EXE)
$(EXE):$(OBJS)
		$(CC) -o $@  $(OBJS) -L $(LIBDIR) $(addprefix -l, $(LIBS));
	#$(RM) $(SOURCEDIR)/*.o $(RM) $(SOURCEDIR)/*.co  $(RM) $(SOURCEDIR)/*.cco
$(SOURCEDIR)/%.o:$(SOURCEDIR)/%.cpp
		$(CC) -c $(CXXFLAGS) $< -o $@ -std=c++11 -I $(INCLUDEDIR)
.PHONY:clean rebuild
clean:
		@$(RM) $(DIR)/*.exe $(RM) $(SOURCEDIR)/*.o $(RM) $(SOURCEDIR)/*.co  $(RM) $(SOURCEDIR)/*.cco 
rebuild: clean start
