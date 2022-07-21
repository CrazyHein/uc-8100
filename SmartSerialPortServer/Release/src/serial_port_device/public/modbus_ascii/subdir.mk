################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/serial_port_device/public/modbus_ascii/modbus_ascii_device.cpp \
../src/serial_port_device/public/modbus_ascii/modbus_ascii_protocol.cpp 

CPP_DEPS += \
./src/serial_port_device/public/modbus_ascii/modbus_ascii_device.d \
./src/serial_port_device/public/modbus_ascii/modbus_ascii_protocol.d 

OBJS += \
./src/serial_port_device/public/modbus_ascii/modbus_ascii_device.o \
./src/serial_port_device/public/modbus_ascii/modbus_ascii_protocol.o 


# Each subdirectory must supply rules for building sources it contributes
src/serial_port_device/public/modbus_ascii/%.o: ../src/serial_port_device/public/modbus_ascii/%.cpp src/serial_port_device/public/modbus_ascii/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-linux-gnueabihf-g++ -I/usr/include/moxa -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-serial_port_device-2f-public-2f-modbus_ascii

clean-src-2f-serial_port_device-2f-public-2f-modbus_ascii:
	-$(RM) ./src/serial_port_device/public/modbus_ascii/modbus_ascii_device.d ./src/serial_port_device/public/modbus_ascii/modbus_ascii_device.o ./src/serial_port_device/public/modbus_ascii/modbus_ascii_protocol.d ./src/serial_port_device/public/modbus_ascii/modbus_ascii_protocol.o

.PHONY: clean-src-2f-serial_port_device-2f-public-2f-modbus_ascii

