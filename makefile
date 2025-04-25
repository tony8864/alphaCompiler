OBJ_DIR = obj

OBJECTS = \
	$(OBJ_DIR)/scanner.o \
	$(OBJ_DIR)/scanner_util.o \
	$(OBJ_DIR)/string_reader.o \
	$(OBJ_DIR)/comment_reader.o \
	$(OBJ_DIR)/parser.o \
	$(OBJ_DIR)/parser_util.o \
	$(OBJ_DIR)/symbol_table.o \
	$(OBJ_DIR)/scope_stack.o \
	$(OBJ_DIR)/scope_space.o \
	$(OBJ_DIR)/quad.o \
	$(OBJ_DIR)/icode.o \
	$(OBJ_DIR)/lc_stack.o

# Original .c file paths
LC_STACK_C = lc_stack/lc_stack.c
QUAD_C = quad/quad.c
ICODE_C = icode/icode.c
PARSER_C_H = parser.tab.c parser.tab.h
PARSER_UTIL_C = parser_util/parser_util.c
SCOPE_SPACE_C = scope_space/scope_space.c
SCOPE_STACK_C = scope_stack/scope_stack.c
SYMBOL_TABLE_C = symbol_table/symbol_table.c
SCANNER_UTIL_C = scanner_util/scanner_util.c
STRING_READER_C = string_reader/string_reader.c
COMMENT_READER_C = comment_reader/comment_reader.c

exe: ${OBJECTS}
	gcc -o exe ${OBJECTS}

# Ensure object directory exists
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/scanner.o: scanner.c | $(OBJ_DIR)
	gcc -c $< -o $@

scanner.c: scanner.l parser.tab.h
	flex --outfile=scanner.c $<

$(OBJ_DIR)/parser.o: parser.tab.c | $(OBJ_DIR)
	gcc -c $< -o $@

${PARSER_C_H}: parser.y
	bison -d $<

$(OBJ_DIR)/scanner_util.o: ${SCANNER_UTIL_C} | $(OBJ_DIR)
	gcc -c $< -o $@

$(OBJ_DIR)/string_reader.o: ${STRING_READER_C} | $(OBJ_DIR)
	gcc -c $< -o $@

$(OBJ_DIR)/comment_reader.o: ${COMMENT_READER_C} | $(OBJ_DIR)
	gcc -c $< -o $@

$(OBJ_DIR)/parser_util.o: ${PARSER_UTIL_C} | $(OBJ_DIR)
	gcc -c $< -o $@

$(OBJ_DIR)/symbol_table.o: ${SYMBOL_TABLE_C} | $(OBJ_DIR)
	gcc -c $< -o $@

$(OBJ_DIR)/scope_stack.o: ${SCOPE_STACK_C} | $(OBJ_DIR)
	gcc -c $< -o $@

$(OBJ_DIR)/scope_space.o: ${SCOPE_SPACE_C} | $(OBJ_DIR)
	gcc -c $< -o $@

$(OBJ_DIR)/icode.o: ${ICODE_C} | $(OBJ_DIR)
	gcc -c $< -o $@

$(OBJ_DIR)/quad.o: ${QUAD_C} | $(OBJ_DIR)
	gcc -c $< -o $@

$(OBJ_DIR)/lc_stack.o: ${LC_STACK_C} | $(OBJ_DIR)
	gcc -c $< -o $@

clean:
	rm -f exe scanner.c ${PARSER_C_H}
	rm -rf $(OBJ_DIR)