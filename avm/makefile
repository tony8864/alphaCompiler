OBJ_DIR = obj

OBJECTS = \
	$(OBJ_DIR)/avm.o \
	$(OBJ_DIR)/loader.o \
	${OBJ_DIR}/dispatcher.o

LOADER_C = loader/loader.c
DISPATCHER_C = dispatcher/dispatcher.c

avm_exe: ${OBJECTS}
	gcc -o avm_exe ${OBJECTS}

${OBJ_DIR}:
	mkdir -p ${OBJ_DIR}

${OBJ_DIR}/avm.o: avm.c | ${OBJ_DIR}
	gcc -c $< -o $@

${OBJ_DIR}/loader.o: ${LOADER_C} | ${OBJ_DIR}
	gcc -c $< -o $@

${OBJ_DIR}/dispatcher.o: ${DISPATCHER_C} | ${OBJ_DIR}
	gcc -c $< -o $@

clean:
	rm -f avm_exe
	rm -rf ${OBJ_DIR}