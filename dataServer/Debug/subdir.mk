################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../lista.cpp \
../server.cpp \
../serverm.cpp \
../sighandlers.cpp 

OBJS += \
./lista.o \
./server.o \
./serverm.o \
./sighandlers.o 

CPP_DEPS += \
./lista.d \
./server.d \
./serverm.d \
./sighandlers.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


