.PHONY: apps tests tools clean

.SECONDARY:

all: tests apps

PLATFORM := $(shell uname)

PAL ?= $(PLATFORM)
BUILD_TYPE ?= Debug

SRC_DIRS := HAP External/HTTP External/JSON External/Base64 PAL PAL/Mock
CFLAGS := -Wall -Werror -DHAP_ENABLE_DEVELOPMENT_ONLY_CODE=1
LDFLAGS :=

BUILD_TYPES := Debug Test Release

# Features applicable to IP based accessories
FEATURES_IP :=

# Features applicable to BLE based accessories
FEATURES_BLE :=

ifdef USE_HW_AUTH
FEATURES_PAL += HAVE_MFI_HW_AUTH
endif

CFLAGS_IP := $(addprefix -D, $(FEATURES_IP) $(FEATURES_PAL))
CFLAGS_BLE := $(addprefix -D, $(FEATURES_BLE) $(FEATURES_PAL))

ifeq ($(LOG_LEVEL),)
	LOG_LEVEL_Debug := 3    # FAULT, ERROR, LOG, INFO, DEBUG
	LOG_LEVEL_Test := 1     # FAULT, ERROR, LOG
	LOG_LEVEL_Release := 0  # no logs
else
	LOG_LEVEL_Debug := $(LOG_LEVEL)
	LOG_LEVEL_Test := $(LOG_LEVEL)
	LOG_LEVEL_Release := $(LOG_LEVEL)
endif

CFLAGS_Debug := -O0 -g -DHAP_LOG_LEVEL=$(LOG_LEVEL_Debug) -DHAP_TESTING
CFLAGS_Test :=  -O0 -g -DHAP_LOG_LEVEL=$(LOG_LEVEL_Test) -DHAP_TESTING
CFLAGS_Release := -O2 -g -DHAP_LOG_LEVEL=$(LOG_LEVEL_Release) -DHAP_DISABLE_ASSERTS=1 -DHAP_DISABLE_PRECONDITIONS=1

OPENSSL_PATH = $(firstword $(wildcard /usr/local/Cellar/openssl@1.1/*))
MBEDTLS_PATH = $(firstword $(wildcard /usr/include/mbedtls) $(wildcard /usr/local/Cellar/mbedtls/*))

-include Build/Makefile.$(PAL)

CFLAGS_OpenSSL := -I$(OPENSSL_PATH)/include
LDFLAGS_OpenSSL := -L$(OPENSSL_PATH)/lib -lcrypto

CFLAGS_MbedTLS := -I$(MBEDTLS_PATH)/include
LDFLAGS_MbedTLS := -L$(MBEDTLS_PATH)/lib -lmbedcrypto

CFLAGS_IP := -DIP=1
CFLAGS_BLE := -DBLE=1

SRC_DIRS += $(SRC_DIRS_$(PAL))
CFLAGS += $(CFLAGS_$(PAL))
LDFLAGS += $(LDFLAGS_$(PAL))
CRYPTO_DIRS += $(CRYPTO_$(PAL))

OUTPUT_DIR := Output/$(PAL)-$(COMPILER)

# Compile against the selected PAL except unit tests, which always use the Mock PAL
CFLAGS_Debug += $(addprefix -I,PAL/$(PAL) $(SRC_DIRS_$(PAL)))
CFLAGS_Test += $(addprefix -I,PAL/Mock)
CFLAGS_Release += $(addprefix -I,PAL/$(PAL) $(SRC_DIRS_$(PAL)))

SRC_DIRS_PAL := $(foreach src,$(SRC_DIRS_$(PAL)),$(src))
CRYPTO_MODULES := $(subst PAL/Crypto/,,$(CRYPTO_DIRS))

CRYPTO ?= $(firstword $(CRYPTO_MODULES))

# We support building C and Objective C code
SRC_EXTS := c m S

# Find all the source files in the given directories
all_sources_in = $(wildcard $(foreach ext,$(SRC_EXTS),$(addsuffix /*.$(ext), $(1))))

# Make each source directory an include path (except PAL implementations)
CFLAGS += $(addprefix -I,$(filter-out PAL/%,$(SRC_DIRS)))

# Add crypto module implementations include paths
CFLAGS += $(foreach crypto,$(CRYPTO_MODULES),$(CFLAGS_$(crypto)))

# Concatenate lists
noop =
space = $(noop) $(noop)
combine = $(subst $(space),$(2),$(1))

# Turn a source list into object file names
to_output = $(addprefix $(OUTPUT_DIR)/$(1)/,$(addsuffix $(3),$(basename $(2))))
to_object = $(call to_output,$(1),$(2),.o)
to_archive = $(call to_output,$(1),$(2),.a)
to_executable = $(call to_output,$(1),$(2),.$(call combine,$(3),.))

# Compile
define compile
$(OUTPUT_DIR)/$(1)/$(2): $(3)
	@mkdir -p $$(dir $$@)
	$(CC) $(CFLAGS) $(CFLAGS_$(1)) $($(subst .,CFLAGS_,$(suffix $3))) $(4) -DHAP_$(1) -c $$< -o $$@

endef

$(eval $(foreach build_type,$(BUILD_TYPES),$(foreach ext,$(SRC_EXTS),$(call compile,$(build_type),%.o,%.$(ext),))))

# Build modules
define build_module_template
$(call to_archive,$(1),$(2)): $(call to_object,$(1),$(3))
	@mkdir -p $$(dir $$@)
	$$(AR) cr $$@ $$^

endef

build_module = $(eval $(foreach build_type,$(BUILD_TYPES),$(call build_module_template,$(build_type),$(1),$(2))))

$(call build_module,sdk,$(SDK_SRCS))
$(call build_module,hap,$(filter-out $(EXCLUDE_$(PAL)),$(call all_sources_in,HAP PAL External/HTTP External/JSON External/Base64)))
$(call build_module,$(PAL),$(filter-out $(EXCLUDE_$(PAL)),$(call all_sources_in,PAL $(SRC_DIRS_PAL))))
$(call build_module,Mock,$(filter-out $(EXCLUDE_$(PAL)),$(call all_sources_in,PAL PAL/Mock)))
$(foreach crypto,$(CRYPTO_MODULES),$(call build_module,$(crypto),$(call all_sources_in,PAL/Crypto/$(crypto))))

CORE = hap $(SDK_$(PAL))

# Link executables
define link_template
$(call to_executable,$(1),$(2),$(3)): $(call to_object,$(1),PAL/HAPPlatformSystemInit.c $(4)) $(call to_archive,$(1),$(5))
	mkdir -p $$(dir $$@)
	$(CC) $(CFLAGS) $(CFLAGS_$(1)) $($(addprefix CFLAGS_,$(3))) -o $$@ $(LINK_BEGIN_$(PAL)) $(LDFLAGS) $(LDFLAGS_$(1)) $($(addprefix LDFLAGS_,$(3))) $$^ $(LINK_END_$(PAL))

endef

build_executable = $(eval $(foreach build_type,$(BUILD_TYPES),$(call link_template,$(build_type),$(1),$(2),$(3),$(4))))

# Build tests
TEST_DIRS := Tests
TEST_SRCS := $(filter-out $(EXCLUDE_$(PAL)),$(call all_sources_in,$(TEST_DIRS)))

# The crypto test has to be run for all crypto backends, the other tests only for the default crypto backend
TESTS = $(foreach crypto,$(CRYPTO_MODULES),$(call to_executable,Test,Tests/HAPCryptoTest,$(crypto))) \
	$(call to_executable,Test,$(filter-out Tests/HAPCryptoTest.c,$(TEST_SRCS)),$(CRYPTO))

$(foreach crypto,$(CRYPTO_MODULES),$(foreach test,$(TEST_SRCS),$(call build_executable,$(test),$(crypto),$(test),$(CORE) Mock $(crypto))))

define run_test
	$(RUN_$(PAL)) $(1)

endef

# Protocols supported on the platform
PROTOCOLS ?= $(PROTOCOLS_$(PAL))

# Build apps
ifeq ($(APPS),)
	APPS_LIST := $(filter-out $(EXCLUDE_$(PAL)),$(subst /.,,$(wildcard Applications/*/.)))
else
	APPS_LIST := $(addprefix Applications/,$(APPS))
endif

$(foreach protocol,$(PROTOCOLS),$(foreach build_type,$(BUILD_TYPES),$(foreach ext,$(SRC_EXTS),$(eval $(call compile,$(build_type),$(protocol)/%.o,%.$(ext),$(CFLAGS_$(protocol)))))))
$(foreach protocol,$(PROTOCOLS),$(foreach app,$(APPS_LIST),$(call build_module,$(protocol)/$(app),$(addprefix $(protocol)/,$(call all_sources_in,$(app))))))
$(foreach protocol,$(PROTOCOLS),$(foreach app,$(APPS_LIST),$(foreach crypto,$(CRYPTO_MODULES),$(call build_executable,$(protocol)/$(app),$(crypto),,$(protocol)/$(app) $(CORE) $(PAL) $(crypto)))))

# Build AccessorySetupGenerator Tool
ACCESSORY_SETUP_GENERATOR:= Tools/AccessorySetupGenerator
$(call build_module,$(ACCESSORY_SETUP_GENERATOR),$(call all_sources_in,$(ACCESSORY_SETUP_GENERATOR)))
$(foreach crypto,$(CRYPTO_MODULES),$(call build_executable,$(ACCESSORY_SETUP_GENERATOR),$(crypto),,$(ACCESSORY_SETUP_GENERATOR) $(CORE) Mock $(crypto)))

info:
	@echo "Compiler: $(COMPILER)"
	@echo "PAL: $(PAL)"
	@echo "Crypto modules: $(CRYPTO_MODULES) (default: $(CRYPTO))"

tests: $(filter-out $(call to_executable,Test,$(addprefix Tests/,$(SKIPPED_TESTS_$(PAL))),$(CRYPTO)),$(TESTS))
	$(foreach test,$^,$(call run_test,$(test)))
	@echo "\nALL TESTS PASSED"

apps: $(foreach protocol,$(PROTOCOLS),$(foreach app,$(APPS_LIST),$(call to_executable,$(BUILD_TYPE),$(protocol)/$(app),$(CRYPTO))))

tools: $(call to_executable,$(BUILD_TYPE),$(ACCESSORY_SETUP_GENERATOR),$(CRYPTO))
ifeq ($(PLATFORM),Darwin)
ifneq ("$(wildcard Tools/JLINK/Makefile)","")
	make OUTPUT_DIR=$(OUTPUT_DIR)/$(BUILD_TYPE)/Tools/JLINK -f Tools/JLINK/Makefile -j 8
endif
endif

clean:
	rm -rf $(OUTPUT_DIR)

check:
	! grep -r "[[:blank:]]$$" HAP PAL External Tests Applications

%.debug:
	$(DEBUGGER) $(basename $@)
