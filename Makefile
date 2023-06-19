EMULATOR := $(shell find . -maxdepth 1 -iname 'rv64gc_emu*' | head -n 1 | xargs basename | tr '[:upper:]' '[:lower:]')
IWAD := $(shell find . -maxdepth 1 -iname '*.wad' | head -n 1 | xargs basename | tr '[:upper:]' '[:lower:]')
OUTPUT := output
DOOM := doom
DOOM_BARE := doom_baremetal
FONT_NAME := font.ttf
FONT_PATH := misc/$(FONT_NAME)

.PHONY: doom_baremetal run_doom_baremetal
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

	@echo "#!/bin/sh\n./$(EMULATOR) --bios $(DOOM_BARE).bin --font $(FONT_NAME)" > $(OUTPUT)/$(DOOM_BARE).sh
	chmod 777 $(OUTPUT)/$(DOOM_BARE).sh

	@echo "Compilation done, run the program with: make run_doom_baremetal"

run_doom_baremetal:
	@cd $(OUTPUT) && ./$(DOOM_BARE).sh

LINUX_DIR := linux
LINUX_BULD_SH := build_linux.sh
IMAGES_DIR := $(LINUX_DIR)/buildroot/output/images
RUN_LINUX_SH := run_linux.sh

.PHONY: linux linux_with_doom_pre linux_with_doom run_linux
linux:
	@if [ -z "$(EMULATOR)" ]; then \
		echo "Please include a built emulator executable in the root of this directory"; \
		exit 1; \
	fi;

	cd $(LINUX_DIR) && ./$(LINUX_BULD_SH);
	@mkdir -p $(OUTPUT);

	@cp $(IMAGES_DIR)/fw_jump.bin $(OUTPUT)/;
	@cp $(IMAGES_DIR)/Image $(OUTPUT)/;

	@dtc -I dts -O dtb $(LINUX_DIR)/dts/dts.dts > $(OUTPUT)/dtb.dtb;

	@cp $(EMULATOR) $(OUTPUT)/;
	@cp $(FONT_PATH) $(OUTPUT)/;

	@echo "#!/bin/sh\n./$(EMULATOR) --bios fw_jump.bin --kernel Image --dtb dtb.dtb --font $(FONT_NAME)" > $(OUTPUT)/$(RUN_LINUX_SH);
	chmod 777 $(OUTPUT)/$(RUN_LINUX_SH);

	@echo "Build done, run linux with: make run_linux";

OVERLAY_DIR=linux/overlay/doom

linux_with_doom_pre:
	@if [ -z "$(EMULATOR)" ]; then \
		echo "Please include a built emulator executable in the root of this directory"; \
		exit 1; \
	fi

	@if [ -z "$(IWAD)" ]; then \
		echo "Please include one .wad file in the root of this directory"; \
		exit 1; \
	fi

	$(MAKE) -C $(DOOM) clean
	@cp $(IWAD) $(DOOM)/

	$(MAKE) -C $(DOOM) iwad
	$(MAKE) -C $(DOOM) linux

	@-rm $(OVERLAY_DIR)
	@mkdir -p $(OVERLAY_DIR)

	@cp $(DOOM)/doomgeneric.elf $(OVERLAY_DIR)/doom

linux_with_doom: linux_with_doom_pre linux

run_linux:
	@cd $(OUTPUT) && ./$(RUN_LINUX_SH)
