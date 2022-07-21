################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/port_scanner/port_scanner.cpp 

CPP_DEPS += \
./src/port_scanner/port_scanner.d 

OBJS += \
./src/port_scanner/port_scanner.o 


# Each subdirectory must supply rules for building sources it contributes
src/port_scanner/%.o: ../src/port_scanner/%.cpp src/port_scanner/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-linux-gnueabihf-g++ -I/usr/include/moxa -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-port_scanner

clean-src-2f-port_scanner:
	-$(RM) ./src/port_scanner/port_scanner.d ./src/port_scanner/port_scanner.o

.PHONY: clean-src-2f-port_scanner

