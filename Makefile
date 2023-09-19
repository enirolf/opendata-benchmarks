CXX:=g++
CXXFLAGS:=$(shell root-config --cflags) -O3
LDFLAGS:=$(shell root-config --libs) -lROOTNTuple

export

TASKSDIR:=$(PWD)/tasks

.PHONY: clean all

all: nanoaod physlite

clean:
	find $(TASKSDIR) -name compiled_nanoaod -delete
	find $(TASKSDIR) -name compiled_physlite -delete

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

# PHYSLITE #####################################################################

physlite: task1_physlite task2_physlite task3_physlite task4_physlite \
				  task5_physlite task6_physlite task7_physlite task8_physlite \

task1_physlite:
	@cd tasks/1 && $(MAKE) compiled_physlite

task2_physlite:
	@cd tasks/2 && $(MAKE) compiled_physlite

task3_physlite:
	@cd tasks/3 && $(MAKE) compiled_physlite

task4_physlite:
	@cd tasks/4 && $(MAKE) compiled_physlite

task5_physlite:
	@cd tasks/5 && $(MAKE) compiled_physlite

task6_physlite:
	@cd tasks/6 && $(MAKE) compiled_physlite

task7_physlite:
	@cd tasks/7 && $(MAKE) compiled_physlite

task8_physlite:
	@cd tasks/8 && $(MAKE) compiled_physlite
