# Use "make TARGET=Linux all" to build for Linux

export

CWD := $(shell pwd)

MAKE := make -f Build/Makefile -j 8
DOCKER := docker
DOCKERFILE := Build/Docker/Dockerfile
RUN := $(DOCKER) run \
  -e PROTOCOLS \
  -e USE_HW_AUTH \
  -e TARGET \
  -e APPS \
  --cap-add=SYS_PTRACE \
  --security-opt seccomp=unconfined \
  --mount type=bind,source="$(CWD)",target=/build \
  -it `make docker`

STEPS := all tests apps clean check info tools %.debug

.PHONY: $(STEPS) %.debug shell docker

HOST := $(shell uname)
TARGET ?= $(HOST)
BUILD_TYPE ?= Debug

ifeq ($(HOST)$(TARGET),LinuxDarwin)
$(error Can't build $(TARGET) on $(HOST).)
endif
ifneq ($(TARGET),Darwin)
	MAKE := $(if $(filter $(HOST),Darwin),$(RUN),) $(MAKE)
endif

ifeq ($(TARGET),Raspi)
	DOCKERFILE := Build/Docker/Dockerfile.Raspi
endif

define make_target
  $(1):
	@$(2) $$@

endef

$(eval $(foreach step,$(STEPS),$(call make_target,$(step),$(MAKE) PAL=$(TARGET) BUILD_TYPE=$(BUILD_TYPE))))

shell:
	@$(RUN) bash

docker:
	@$(DOCKER) build -f $(DOCKERFILE) . | tee /dev/stderr | grep "Successfully built" | cut -d ' ' -f 3
