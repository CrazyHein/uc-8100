################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/utility/pugixml/pugixml.cpp 

CPP_DEPS += \
./src/utility/pugixml/pugixml.d 

OBJS += \
./src/utility/pugixml/pugixml.o 


# Each subdirectory must supply rules for building sources it contributes
src/utility/pugixml/%.o: ../src/utility/pugixml/%.cpp src/utility/pugixml/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-linux-gnueabihf-g++ -I/usr/include/moxa -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-utility-2f-pugixml

clean-src-2f-utility-2f-pugixml:
	-$(RM) ./src/utility/pugixml/pugixml.d ./src/utility/pugixml/pugixml.o

.PHONY: clean-src-2f-utility-2f-pugixml

