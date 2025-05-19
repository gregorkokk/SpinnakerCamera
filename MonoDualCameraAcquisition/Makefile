# Project settings
PROJECT_ROOT = ../../
OPT_INC = ${PROJECT_ROOT}/common/make/common_spin.mk
-include ${OPT_INC}

# Compiler and flags
CFLAGS = -std=c++11 -Wall -D LINUX
CXX = g++ ${CFLAGS}

# Directories
SDIR = .
ODIR = .obj/build
BIN = ../../bin
MKDIR = mkdir -p

# Output binary
OUTPUTNAME = two_cam_test

# Source and object files
SRC_FILES = $(wildcard ${SDIR}/*.cpp)
# In case we may need to adjust the order in which object files are passed to the linker
#SRC_FILES = camera_settings.cpp camera_manager.cpp main.cpp
OBJ = $(patsubst %.cpp,${ODIR}/%.o,$(notdir ${SRC_FILES}))

# Spinnaker dependencies
INC = -I../../include -I/usr/local/include/spinnaker
LIB = -L../../lib -lSpinnaker -Wl,-rpath ../../lib/

# Rules/recipes & Final binary
${OUTPUTNAME}: ${OBJ}
	${CXX} -o ${OUTPUTNAME} ${OBJ} ${LIB}
	mv ${OUTPUTNAME} ${BIN}

# Intermediate object files
${OBJ}: ${ODIR}/%.o : ${SDIR}/%.cpp
	@${MKDIR} ${ODIR}
	${CXX} ${CFLAGS} ${INC} -Wall -D LINUX -c $< -o $@

# Clean up intermediate objects
clean_obj:
	rm -f ${OBJ}
	@echo "intermediate objects cleaned up!"

# Clean up everything.
clean: clean_obj
	rm -f ${OUTDIR}/${OUTPUTNAME}
	@echo "all cleaned up!"
