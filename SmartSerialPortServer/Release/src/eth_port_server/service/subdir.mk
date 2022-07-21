################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/eth_port_server/service/sde_server.cpp 

CPP_DEPS += \
./src/eth_port_server/service/sde_server.d 

OBJS += \
./src/eth_port_server/service/sde_server.o 


# Each subdirectory must supply rules for building sources it contributes
src/eth_port_server/service/%.o: ../src/eth_port_server/service/%.cpp src/eth_port_server/service/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-linux-gnueabihf-g++ -I/usr/include/moxa -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-eth_port_server-2f-service

clean-src-2f-eth_port_server-2f-service:
	-$(RM) ./src/eth_port_server/service/sde_server.d ./src/eth_port_server/service/sde_server.o

.PHONY: clean-src-2f-eth_port_server-2f-service

