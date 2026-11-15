MODULE_NAME = lutjpeg
TOP_DIR = ../..
MODULE_OBJS = $(BUILD_DIR)/lutjpeg.o $(BUILD_DIR)/catalog.o $(BUILD_DIR)/fos_dev.o $(BUILD_DIR)/fos_jpeg.o $(BUILD_DIR)/fos_print.o $(BUILD_DIR)/fos_looks.o $(BUILD_DIR)/lut-libm.a $(TOP_DIR)/platform/600D.102/build/gcc-libgcc.a
MODULE_CFLAGS += -Iupstream -DCONFIG_600D=1
include $(TOP_DIR)/modules/Makefile
.DEFAULT_GOAL := $(BUILD_DIR)/module_complete
$(BUILD_DIR)/fos_%.o: upstream/fos_%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CDEPS_FLAGS) -c -o $@ $<
$(BUILD_DIR)/lut-libm.a: $(TOP_DIR)/platform/600D.102/build/newlib-libm.a | $(BUILD_DIR)
	$(OBJCOPY) --redefine-sym __errno=lut_errno $< $@

$(filter %.o,$(MODULE_OBJS)): Makefile
