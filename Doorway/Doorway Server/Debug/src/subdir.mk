################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/server.c 

OBJS += \
./src/server.o 

C_DEPS += \
./src/server.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -m32 -I/usr/include/mysql -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


