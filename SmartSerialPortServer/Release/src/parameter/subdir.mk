################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/parameter/startup_parameter.cpp 

CPP_DEPS += \
./src/parameter/startup_parameter.d 

OBJS += \
./src/parameter/startup_parameter.o 


# Each subdirectory must supply rules for building sources it contributes
src/parameter/%.o: ../src/parameter/%.cpp src/parameter/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-linux-gnueabihf-g++ -I/usr/include/moxa -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-parameter

clean-src-2f-parameter:
	-$(RM) ./src/parameter/startup_parameter.d ./src/parameter/startup_parameter.o

.PHONY: clean-src-2f-parameter

