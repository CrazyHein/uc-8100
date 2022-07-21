################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/serial_port_device/public/modbus_rtu/modbus_rtu_device.cpp \
../src/serial_port_device/public/modbus_rtu/modbus_rtu_protocol.cpp 

CPP_DEPS += \
./src/serial_port_device/public/modbus_rtu/modbus_rtu_device.d \
./src/serial_port_device/public/modbus_rtu/modbus_rtu_protocol.d 

OBJS += \
./src/serial_port_device/public/modbus_rtu/modbus_rtu_device.o \
./src/serial_port_device/public/modbus_rtu/modbus_rtu_protocol.o 


# Each subdirectory must supply rules for building sources it contributes
src/serial_port_device/public/modbus_rtu/%.o: ../src/serial_port_device/public/modbus_rtu/%.cpp src/serial_port_device/public/modbus_rtu/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-linux-gnueabihf-g++ -I/usr/include/moxa -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-serial_port_device-2f-public-2f-modbus_rtu

clean-src-2f-serial_port_device-2f-public-2f-modbus_rtu:
	-$(RM) ./src/serial_port_device/public/modbus_rtu/modbus_rtu_device.d ./src/serial_port_device/public/modbus_rtu/modbus_rtu_device.o ./src/serial_port_device/public/modbus_rtu/modbus_rtu_protocol.d ./src/serial_port_device/public/modbus_rtu/modbus_rtu_protocol.o

.PHONY: clean-src-2f-serial_port_device-2f-public-2f-modbus_rtu

