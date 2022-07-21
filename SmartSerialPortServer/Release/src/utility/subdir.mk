################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/utility/moxa_led.cpp \
../src/utility/moxa_serial_port.cpp \
../src/utility/socket.cpp 

CPP_DEPS += \
./src/utility/moxa_led.d \
./src/utility/moxa_serial_port.d \
./src/utility/socket.d 

OBJS += \
./src/utility/moxa_led.o \
./src/utility/moxa_serial_port.o \
./src/utility/socket.o 


# Each subdirectory must supply rules for building sources it contributes
src/utility/%.o: ../src/utility/%.cpp src/utility/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-linux-gnueabihf-g++ -I/usr/include/moxa -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-utility

clean-src-2f-utility:
	-$(RM) ./src/utility/moxa_led.d ./src/utility/moxa_led.o ./src/utility/moxa_serial_port.d ./src/utility/moxa_serial_port.o ./src/utility/socket.d ./src/utility/socket.o

.PHONY: clean-src-2f-utility

