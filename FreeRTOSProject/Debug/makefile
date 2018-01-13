################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Source/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: FreeRTOSProject.axf

# Tool invocations
FreeRTOSProject.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nostdlib -L"O:\good version\DFRobotRTOS_T2_S1\LibFreeRTOS\Debug" -L"O:\good version\DFRobotRTOS_T2_S1\LibLPC17xx\Debug" -L"O:\good version\DFRobotRTOS_T2_S1\LibCMSIS\Debug" -Xlinker -Map="FreeRTOSProject.map" -Xlinker --gc-sections -mcpu=cortex-m3 -mthumb -T "FreeRTOSProject_Debug.ld" -o "FreeRTOSProject.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) FreeRTOSProject.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size "FreeRTOSProject.axf"; # arm-none-eabi-objcopy -O binary "FreeRTOSProject.axf" "FreeRTOSProject.bin" ; checksum -p LPC1769 -d "FreeRTOSProject.bin";
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
