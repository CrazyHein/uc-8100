################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/eth_port_server/protocol/simple_data_exchanger.cpp 

CPP_DEPS += \
./src/eth_port_server/protocol/simple_data_exchanger.d 

OBJS += \
./src/eth_port_server/protocol/simple_data_exchanger.o 


# Each subdirectory must supply rules for building sources it contributes
src/eth_port_server/protocol/%.o: ../src/eth_port_server/protocol/%.cpp src/eth_port_server/protocol/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-linux-gnueabihf-g++ -I/usr/include/moxa -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-eth_port_server-2f-protocol

clean-src-2f-eth_port_server-2f-protocol:
	-$(RM) ./src/eth_port_server/protocol/simple_data_exchanger.d ./src/eth_port_server/protocol/simple_data_exchanger.o

.PHONY: clean-src-2f-eth_port_server-2f-protocol

