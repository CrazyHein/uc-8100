################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/common/exception.cpp 

CPP_DEPS += \
./src/common/exception.d 

OBJS += \
./src/common/exception.o 


# Each subdirectory must supply rules for building sources it contributes
src/common/%.o: ../src/common/%.cpp src/common/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-linux-gnueabihf-g++ -I/usr/include/moxa -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-common

clean-src-2f-common:
	-$(RM) ./src/common/exception.d ./src/common/exception.o

.PHONY: clean-src-2f-common

