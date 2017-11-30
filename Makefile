# This is a custom Makefile created to speed up the developing process, it will not be included in the final release
# Author: Daniele Paolini, daniele.paolini@hotmail.it

.PHONY = all clean

CC = icpc

CFLAGS = -O3 -Wall -pedantic -pthread -std=c++11 -I /home/dan/fastflow

DIR_TEST = @if [ ! -d "test/bin" ]; then mkdir test/bin ; fi 

all: basic_test pipeline_test pipeline_nested_test farm_test farm_complex_test 

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
	@echo "Removing executables..."
	-@rm -rf test/bin
	@echo "Done!"
