OBJ_DIR = obj

OBJECTS = \
	${OBJ_DIR}/avm.o \
	${OBJ_DIR}/loader.o \
	${OBJ_DIR}/memory.o \
	${OBJ_DIR}/dispatcher.o \
	${OBJ_DIR}/arithmetic.o \
	${OBJ_DIR}/assign.o

ASSIGN_EXE_C = executors/assign.c
ARITHMETIC_EXE_C = executors/arithmetic.c
LOADER_C = loader/loader.c
MEMORY_C = memory/memory.c
DISPATCHER_C = dispatcher/dispatcher.c

avm: ${OBJECTS}
	gcc -o avm ${OBJECTS}

${OBJ_DIR}:
	mkdir -p ${OBJ_DIR}

${OBJ_DIR}/avm.o: avm.c | ${OBJ_DIR}
	gcc -c $< -o $@

${OBJ_DIR}/loader.o: ${LOADER_C} | ${OBJ_DIR}
	gcc -c $< -o $@

${OBJ_DIR}/memory.o: ${MEMORY_C} | ${OBJ_DIR}
	gcc -c $< -o $@

${OBJ_DIR}/dispatcher.o: ${DISPATCHER_C} | ${OBJ_DIR}
	gcc -c $< -o $@

${OBJ_DIR}/arithmetic.o: ${ARITHMETIC_EXE_C} | ${OBJ_DIR}
	gcc -c $< -o $@

${OBJ_DIR}/assign.o: ${ASSIGN_EXE_C} | ${OBJ_DIR}
	gcc -c $< -o $@

clean:
	rm -f avm
	rm -rf ${OBJ_DIR}