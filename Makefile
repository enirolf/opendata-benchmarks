CXX:=g++
CXXFLAGS:=$(shell root-config --cflags) -O3
LDFLAGS:=$(shell root-config --libs) -lROOTNTuple

export

TASKSDIR:=$(PWD)/tasks

.PHONY: clean all

all: clear_page_cache nanoaod

clean:
	find $(TASKSDIR) -name compiled_nanoaod -delete

clear_page_cache: clear_page_cache.cxx
	$(CXX) -Wall -g -o $@ $^
	sudo chown root $@
	sudo chmod 4755 $@

# NanoAOD ######################################################################

nanoaod: task1_nanoaod task2_nanoaod task3_nanoaod task4_nanoaod \
				 task5_nanoaod task6_nanoaod task7_nanoaod task8_nanoaod

task1_nanoaod:
	@cd tasks/1 && $(MAKE) compiled_nanoaod

task2_nanoaod:
	@cd tasks/2 && $(MAKE) compiled_nanoaod

task3_nanoaod:
	@cd tasks/3 && $(MAKE) compiled_nanoaod

task4_nanoaod:
	@cd tasks/4 && $(MAKE) compiled_nanoaod

task5_nanoaod:
	@cd tasks/5 && $(MAKE) compiled_nanoaod

task6_nanoaod:
	@cd tasks/6 && $(MAKE) compiled_nanoaod

task7_nanoaod:
	@cd tasks/7 && $(MAKE) compiled_nanoaod

task8_nanoaod:
	@cd tasks/8 && $(MAKE) compiled_nanoaod
