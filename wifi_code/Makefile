# Name of your application
APPLICATION = HomeDefender

# Path to the RIOT base directory
RIOTBASE ?= $(CURDIR)/../../RIOT

# Specify the board you are using
BOARD ?= esp32-heltec-lora32-v2

# Include necessary modules
USEMODULE += xtimer
USEMODULE += periph_gpio

USEMODULE += shell
USEMODULE += ps

# This has to be the absolute path to the RIOT-OS base directory
DEVELHELP ?= 1

FEATURES_REQUIRED += periph_gpio periph_i2c

# Include the main RIOT makefile
include $(RIOTBASE)/Makefile.include
