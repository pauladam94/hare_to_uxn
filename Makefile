.DELETE_ON_ERROR: # don't create file when there is an error
MAKEFLAGS += --warn-undefined-variables
MAKEFLAGS += --no-builtin-rules
CC = clang
CFLAGS = -Wall -Wextra -Wpedantic -fshort-enums -g
# Other flag : -Werror -g

# BUILD EVERYTHING
build: bin/test_all

rename:
	for dir in $(wildcard test/*); do mv $$dir/code $$dir/main.ha; done

# TEST
test: clean_results bin/test_all $(wildcard test/*/*_expected)
	@start_time=$$(date +%s); \
	for dir in $(wildcard test/*); do \
		./bin/test_all $$dir; \
	done; \
	end_time=$$(date +%s); \
	elapsed_time=$$((end_time - start_time)); \
	echo "Time doing test: $$elapsed_time s"

# add bin/uxncli or not
bin/test_all: test.c bin/ bin/lexer.o bin/parser.o bin/compiler.o bin/compiler_utils.o bin/colors.o bin/files.o
	@$(CC) $(CFLAGS) test.c bin/lexer.o bin/parser.o bin/compiler.o bin/compiler_utils.o bin/colors.o bin/files.o -o bin/test_all

# LEXER
bin/lexer.o: lexer/lexer.c lexer/lexer.h
	@$(CC) $(CFLAGS) -c lexer/lexer.c -o bin/lexer.o

# PARSER
bin/parser.o: bin/lexer.o parser/parser.c parser/parser.h
	@$(CC) $(CFLAGS) -c parser/parser.c -o bin/parser.o

# COMPILER
bin/compiler.o: bin/lexer.o bin/parser.o bin/compiler_utils.o compiler_to_uxn/compiler.c compiler_to_uxn/compiler.h
	@$(CC) $(CFLAGS) -c compiler_to_uxn/compiler.c -o bin/compiler.o

bin/compiler_utils.o: compiler_to_uxn/compiler_utils.c compiler_to_uxn/compiler_utils.h
	@$(CC) $(CFLAGS) -c compiler_to_uxn/compiler_utils.c -o bin/compiler_utils.o

# UTILS
bin/colors.o: utils/colors.c utils/colors.h
	@$(CC) $(CFLAGS) -c utils/colors.c -o bin/colors.o

bin/files.o: utils/files.c utils/files.h
	@$(CC) $(CFLAGS) -c utils/files.c -o bin/files.o

# bin directory
bin/:
	mkdir bin

# CLEAN
clean: clean_binaries clean_results

clean_binaries:
	@rm -f $(wildcard bin/*)

clean_results:
	@rm -f $(wildcard test/*/error)
	@rm -f $(wildcard test/*/code.rom)
	@rm -f $(wildcard test/*/code.rom.sym)
	@rm -f $(wildcard test/*/code.uxntal)
	@rm -f $(wildcard test/*/*result)
