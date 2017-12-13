# This is a custom Makefile created to speed up the developing process, it will not be included in the final release
# Author: Daniele Paolini, daniele.paolini@hotmail.it

# NOTE: remember to update $PATH with icpc location before make anything (source /opt/intel/INSTALL_DIR/bin/psxevars.sh)
# where INSTALL_DIR is the directory in wich the binaries of Intel Compiler is contained. As alternative of using icpc
# consider to use g++ by simply modify the CC variable to g++.

# IMPORTANT: remember to change FFDIR variable to the actual path of FastFlow headers

# NOTE: I use icpc to compile this code, so using g++ may lead to some not critical compiler warnings, feel free to
# modify CFLAGS variable in order to suit your needs.

.PHONY = all clean

CC = icpc
FFDIR = /home/dan/fastflow

CFLAGS = -O3 -Wall -pedantic -pthread -std=c++11 -I $(FFDIR)

DIR_TEST = @if [ ! -d "test/bin" ]; then mkdir test/bin ; fi 

all: basic_test pipeline_test pipeline_nested_test farm_test farm_complex_test comp_benchmark

basic_test: test/basic_test.cpp
	$(DIR_TEST)
	@echo "Compiling basic_test sources..."
	@$(CC) $(CFLAGS) test/basic_test.cpp -o test/bin/basic_test
	@echo "Done!"
	@test/bin/basic_test
	@echo ""

farm_complex_test: test/farm_complex_test.cpp
	$(DIR_TEST)
	@echo "Compiling farm_complex_test sources..."
	@$(CC) $(CFLAGS) test/farm_complex_test.cpp -o test/bin/farm_complex_test
	@echo "Done!"
	@test/bin/farm_complex_test
	@echo ""

farm_test: test/farm_test.cpp
	$(DIR_TEST)
	@echo "Compiling farm_test sources..."
	@$(CC) $(CFLAGS) test/farm_test.cpp -o test/bin/farm_test
	@echo "Done!"
	@test/bin/farm_test
	@echo ""

comp_benchmark: test/comp_benchmark.cpp
	$(DIR_TEST)
	@echo "Compiling comp_benchmark sources..."
	@$(CC) $(CFLAGS) test/comp_benchmark.cpp -o test/bin/comp_benchmark
	@echo "Done!"
	@echo "Run this benchmark with \"test/comp_benchmark.sh\""
	@test/bin/comp_benchmark -h
	@echo ""

pipeline_nested_test: test/pipeline_nested_test.cpp
	$(DIR_TEST)
	@echo "Compiling pipeline_nested_test sources..."
	@$(CC) $(CFLAGS) test/pipeline_nested_test.cpp -o test/bin/pipeline_nested_test
	@echo "Done!"
	@test/bin/pipeline_nested_test
	@echo ""

pipeline_test: test/pipeline_test.cpp
	$(DIR_TEST)
	@echo "Compiling pipeline_test sources..."
	@$(CC) $(CFLAGS) test/pipeline_test.cpp -o test/bin/pipeline_test
	@echo "Done!"
	@test/bin/pipeline_test
	@echo ""

clean:
	@echo "Removing binaries..."
	-@rm -rf test/bin
	@echo "Done!"
