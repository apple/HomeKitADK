# Use "make TARGET=Linux all" to build for Linux

export

CWD := $(shell pwd)

MAKE := make -f Build/Makefile -j 8
DOCKER := docker
DOCKERFILE := Build/Docker/Dockerfile

ENABLE_TTY =
MAKE_DOCKER = $(DOCKER) build -f $(DOCKERFILE) . | tee /dev/stderr | grep "Successfully built" | cut -d ' ' -f 3
RUN = $(DOCKER) run \
  -e APPS \
  -e BUILD_TYPE \
  -e HOST \
  -e LOG_LEVEL \
  -e PROTOCOLS \
  -e TARGET \
  -e USE_HW_AUTH \
  -e USE_NFC \
  --cap-add=SYS_PTRACE \
  --security-opt seccomp=unconfined \
  --mount type=bind,source="$(CWD)",target=/build \
  -i $(ENABLE_TTY) `$(MAKE_DOCKER)`

STEPS := all tests apps clean check info tools %.debug

.PHONY: $(STEPS) %.debug shell docker

HOST := $(shell uname)
TARGET ?= $(HOST)
BUILD_TYPE ?= Debug

ifeq ($(HOST)$(TARGET),LinuxDarwin)
$(error Can't build $(TARGET) on $(HOST).)
endif
ifneq ($(TARGET),Darwin)
	ifneq (,$(wildcard /.dockerenv))
		# If we are already running inside docker
		MAKE := $(MAKE)
	else
		# Else run make inside docker
		MAKE := $(RUN) $(MAKE)
	endif
endif

ifeq ($(TARGET),Raspi)
	DOCKERFILE := Build/Docker/Dockerfile.Raspi
endif

define make_target
  $(1):
	@$(2) $$@

endef

$(eval $(foreach step,$(STEPS),$(call make_target,$(step),$(MAKE) PAL=$(TARGET))))

shell: ENABLE_TTY=-t
shell:
	@$(RUN) bash

docker:
	@$(MAKE_DOCKER)
