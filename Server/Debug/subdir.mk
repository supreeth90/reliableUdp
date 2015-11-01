################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Datagram.cpp \
../Logger.cpp \
../MainClass.cpp \
../PacketStats.cpp \
../SlidingWindow.cpp \
../SlidingWindowBuffer.cpp \
../UdpServer.cpp 

OBJS += \
./Datagram.o \
./Logger.o \
./MainClass.o \
./PacketStats.o \
./SlidingWindow.o \
./SlidingWindowBuffer.o \
./UdpServer.o 

CPP_DEPS += \
./Datagram.d \
./Logger.d \
./MainClass.d \
./PacketStats.d \
./SlidingWindow.d \
./SlidingWindowBuffer.d \
./UdpServer.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


