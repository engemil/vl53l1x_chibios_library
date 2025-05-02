# Base path for the VL53L1X API driver
VL53L1X_ULD_BASE := $(LIBS)/vl53l1x_uld

# List of all Source files
LIBSSRC := $(wildcard $(VL53L1X_ULD_BASE)/core/*.c) \
           $(wildcard $(VL53L1X_ULD_BASE)/platform/*.c)

# Required include directories
LIBSINC := $(VL53L1X_ULD_BASE)/core/ \
           $(VL53L1X_ULD_BASE)/platform/

# Add to shared variables for ChibiOS build system
ALLCSRC += $(LIBSSRC)
ALLINC  += $(LIBSINC)
