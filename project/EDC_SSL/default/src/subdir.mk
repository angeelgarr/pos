################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/BmpToPrnLogo.c \
../src/ClssProc.c \
../src/EmvMid.c \
../src/MultiApp.c \
../src/ProcTrans.c \
../src/Simulate.c \
../src/SxxCom.c \
../src/appLib.c \
../src/at_command.c \
../src/checkopt.c \
../src/commlib.c \
../src/commproc.c \
../src/cpinpad.c \
../src/debug.c \
../src/emvproc.c \
../src/emvtest.c \
../src/eventmsg.c \
../src/fileoper.c \
../src/global.c \
../src/initpos.c \
../src/lng.c \
../src/main.c \
../src/manage.c \
../src/mlogo.c \
../src/pack8583.c \
../src/password.c \
../src/print.c \
../src/setup.c \
../src/ssl_wrapper.c \
../src/st8583.c \
../src/tranproc.c \
../src/util.c 

OBJS += \
./src/BmpToPrnLogo.o \
./src/ClssProc.o \
./src/EmvMid.o \
./src/MultiApp.o \
./src/ProcTrans.o \
./src/Simulate.o \
./src/SxxCom.o \
./src/appLib.o \
./src/at_command.o \
./src/checkopt.o \
./src/commlib.o \
./src/commproc.o \
./src/cpinpad.o \
./src/debug.o \
./src/emvproc.o \
./src/emvtest.o \
./src/eventmsg.o \
./src/fileoper.o \
./src/global.o \
./src/initpos.o \
./src/lng.o \
./src/main.o \
./src/manage.o \
./src/mlogo.o \
./src/pack8583.o \
./src/password.o \
./src/print.o \
./src/setup.o \
./src/ssl_wrapper.o \
./src/st8583.o \
./src/tranproc.o \
./src/util.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC Compiler'
	"D:/pax/pax_tool/prolin_sdk_win-2.8.13/sdk/toolchains/arm-4.4.1/bin/arm-none-linux-gnueabi-gcc" -O0 -g2 -Wall -funwind-tables -I"D:/pax/2.8.13_prolin/EDC_SSL/inc" -I"D:/pax/2.8.13_prolin/EDC_SSL/src" -I"D:/pax/2.8.13_prolin/EDC_SSL/src/MID" -I"D:/pax/pax_tool/prolin_sdk_win-2.8.13/sdk/platforms/paxngfp_201205/include" -I"D:/pax/pax_tool/prolin_sdk_win-2.8.13/sdk/platforms/paxngfp_201205/include/freetype2" -I"D:/pax/pax_tool/prolin_sdk_win-2.8.13/sdk/toolchains/arm-4.4.1/arm-none-linux-gnueabi/libc/usr/include" -I"D:/pax/pax_tool/prolin_sdk_win-2.8.13/sdk/toolchains/arm-4.4.1/lib/gcc/arm-none-linux-gnueabi/4.6.3/include" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


