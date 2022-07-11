################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/serial_port_device/port.cpp 

CPP_DEPS += \
./src/serial_port_device/port.d 

OBJS += \
./src/serial_port_device/port.o 


# Each subdirectory must supply rules for building sources it contributes
src/serial_port_device/%.o: ../src/serial_port_device/%.cpp src/serial_port_device/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-linux-gnueabihf-g++ -I/usr/include/moxa -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-serial_port_device

clean-src-2f-serial_port_device:
	-$(RM) ./src/serial_port_device/port.d ./src/serial_port_device/port.o

.PHONY: clean-src-2f-serial_port_device

