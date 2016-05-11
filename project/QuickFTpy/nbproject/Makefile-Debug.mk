#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/src/base64.o \
	${OBJECTDIR}/src/client.o \
	${OBJECTDIR}/src/file.o \
	${OBJECTDIR}/src/gz.o \
	${OBJECTDIR}/src/list.o \
	${OBJECTDIR}/src/logger.o \
	${OBJECTDIR}/src/message.o \
	${OBJECTDIR}/src/mutex.o \
	${OBJECTDIR}/src/process.o \
	${OBJECTDIR}/src/py.o \
	${OBJECTDIR}/src/server.o \
	${OBJECTDIR}/src/socket.o \
	${OBJECTDIR}/src/string.o \
	${OBJECTDIR}/src/thread.o \
	${OBJECTDIR}/src/time.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/quickftpy.${CND_DLIB_EXT}

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/quickftpy.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.c} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/quickftpy.${CND_DLIB_EXT} ${OBJECTFILES} ${LDLIBSOPTIONS} -lpthread -lz -lm -lpython2.7 -shared -fPIC

${OBJECTDIR}/src/base64.o: src/base64.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/base64.o src/base64.c

${OBJECTDIR}/src/client.o: src/client.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/client.o src/client.c

${OBJECTDIR}/src/file.o: src/file.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/file.o src/file.c

${OBJECTDIR}/src/gz.o: src/gz.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/gz.o src/gz.c

${OBJECTDIR}/src/list.o: src/list.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/list.o src/list.c

${OBJECTDIR}/src/logger.o: src/logger.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/logger.o src/logger.c

${OBJECTDIR}/src/message.o: src/message.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/message.o src/message.c

${OBJECTDIR}/src/mutex.o: src/mutex.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/mutex.o src/mutex.c

${OBJECTDIR}/src/process.o: src/process.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/process.o src/process.c

${OBJECTDIR}/src/py.o: src/py.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/py.o src/py.c

${OBJECTDIR}/src/server.o: src/server.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/server.o src/server.c

${OBJECTDIR}/src/socket.o: src/socket.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/socket.o src/socket.c

${OBJECTDIR}/src/string.o: src/string.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/string.o src/string.c

${OBJECTDIR}/src/thread.o: src/thread.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/thread.o src/thread.c

${OBJECTDIR}/src/time.o: src/time.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/time.o src/time.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/quickftpy.${CND_DLIB_EXT}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
