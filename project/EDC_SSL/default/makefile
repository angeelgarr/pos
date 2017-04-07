################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
NOSTRIP += \
app.nostrip \


# All Target
all: EDC_SSL

# Tool invocations
EDC_SSL: $(NOSTRIP)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC Strip'
	"D:/pax/pax_tool/prolin_sdk_win-2.8.13/sdk/toolchains/arm-4.4.1/bin/arm-none-linux-gnueabi-strip" -g $(NOSTRIP) -o"EDC_SSL"
	@echo 'Finished building target: $@'
	@echo ' '

app.nostrip: $(OBJS) $(USER_OBJS)
	@echo 'Invoking: GCC Linker'
	"D:/pax/pax_tool/prolin_sdk_win-2.8.13/sdk/toolchains/arm-4.4.1/bin/arm-none-linux-gnueabi-gcc" -o"app.nostrip" $(OBJS) $(USER_OBJS) $(LIBS) -L"D:/pax/pax_tool/prolin_sdk_win-2.8.13/sdk/platforms/paxngfp_201205/lib" -losal -Wl,-rpath=//opt/lib -Wl,-rpath=./lib -Wl,-rpath-link,"D:/pax/pax_tool/prolin_sdk_win-2.8.13/sdk/platforms/paxngfp_201205/lib" -lMID -lcrypto -lAE_Prolin_v202 -lD_DPAS_LIB_v100 -lD_EMV_LIB_v603 -lD_ENTRY_LIB_v505 -lD_JCB_LIB_v101 -lD_MC_LIB_v450 -lD_WAVE_LIB_v305 -lDevice_Prolin_v503 -lEG_PROLIN2_4_V0.29 -losal2 -lProTimsAPI -lcc2564_wlt -lfreetype -liniparser -lpng -lpthread -lts -lssl -lxui -L"D:/pax/2.8.13_prolin/EDC_SSL/lib" -L"D:/pax/2.8.13_prolin/EDC_SSL/lib" -L"D:/pax/2.8.13_prolin/EDC_SSL/lib_a"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(NOSTRIP) EDC_SSL
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
