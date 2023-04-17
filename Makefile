EMULATOR := $(shell find . -maxdepth 1 -iname 'rv64gc_emu*' | head -n 1 | xargs basename | tr '[:upper:]' '[:lower:]')
IWAD := $(shell find . -maxdepth 1 -iname '*.wad' | head -n 1 | xargs basename | tr '[:upper:]' '[:lower:]')
OUTPUT := output
DOOM := doom
DOOM_BARE := doom_baremetal
FONT_NAME := font.ttf
FONT_PATH := misc/$(FONT_NAME)

.PHONY: doom_baremetal
doom_baremetal:
	@if [ -z "$(EMULATOR)" ]; then \
		echo "Please include a built emulator executable in the root of this directory"; \
		exit 1; \
	fi

	@if [ -z "$(IWAD)" ]; then \
		echo "Please include one .wad file in the root of this directory"; \
		exit 1; \
	fi

	@cp $(IWAD) $(DOOM)/

	$(MAKE) -C $(DOOM) iwad
	$(MAKE) -C $(DOOM) all

	@mkdir -p $(OUTPUT)

	@cp $(DOOM)/doomgeneric.bin $(OUTPUT)/$(DOOM_BARE).bin
	@cp $(EMULATOR) $(OUTPUT)/
	@cp $(FONT_PATH) $(OUTPUT)/

	@echo "./$(EMULATOR) --bios $(DOOM_BARE).bin --font $(FONT_NAME)" > $(OUTPUT)/$(DOOM_BARE).sh

	@echo "Compilation done, run the program with the following command: \"cd $(OUTPUT) && ./$(DOOM_BARE).sh\""
