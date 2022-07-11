################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/eth_port_server/memory/generic_shared_memory.cpp 

CPP_DEPS += \
./src/eth_port_server/memory/generic_shared_memory.d 

OBJS += \
./src/eth_port_server/memory/generic_shared_memory.o 


# Each subdirectory must supply rules for building sources it contributes
src/eth_port_server/memory/%.o: ../src/eth_port_server/memory/%.cpp src/eth_port_server/memory/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-linux-gnueabihf-g++ -I/usr/include/moxa -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-eth_port_server-2f-memory

clean-src-2f-eth_port_server-2f-memory:
	-$(RM) ./src/eth_port_server/memory/generic_shared_memory.d ./src/eth_port_server/memory/generic_shared_memory.o

.PHONY: clean-src-2f-eth_port_server-2f-memory

