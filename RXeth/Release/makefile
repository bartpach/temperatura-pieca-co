################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
LSS += \
RXeth.lss \

FLASH_IMAGE += \
RXeth.hex \

EEPROM_IMAGE += \
RXeth.eep \

SIZEDUMMY += \
sizedummy \


# All Target
all: RXeth.elf secondary-outputs

# Tool invocations
RXeth.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: AVR C Linker'
	avr-gcc -Wl,-Map,RXeth.map -mmcu=atmega328p -o "RXeth.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

RXeth.lss: RXeth.elf
	@echo 'Invoking: AVR Create Extended Listing'
	-avr-objdump -h -S RXeth.elf  >"RXeth.lss"
	@echo 'Finished building: $@'
	@echo ' '

RXeth.hex: RXeth.elf
	@echo 'Create Flash image (ihex format)'
	-avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex RXeth.elf  "RXeth.hex"
	@echo 'Finished building: $@'
	@echo ' '

RXeth.eep: RXeth.elf
	@echo 'Create eeprom image (ihex format)'
	-avr-objcopy -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 -O ihex RXeth.elf  "RXeth.eep"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: RXeth.elf
	@echo 'Invoking: Print Size'
	-avr-size --format=avr --mcu=atmega328p RXeth.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(FLASH_IMAGE)$(ELFS)$(OBJS)$(ASM_DEPS)$(EEPROM_IMAGE)$(S_DEPS)$(SIZEDUMMY)$(S_UPPER_DEPS)$(LSS)$(C_DEPS) RXeth.elf
	-@echo ' '

secondary-outputs: $(LSS) $(FLASH_IMAGE) $(EEPROM_IMAGE) $(SIZEDUMMY)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
