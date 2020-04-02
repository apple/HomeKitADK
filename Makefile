# Use "make TARGET=Linux all" to build for Linux

export

STEPS := all tests apps clean check info tools docs %.debug
.PHONY: $(STEPS) %.debug shell docker

CWD := $(shell pwd)
HOST := $(shell uname)

NPROC :=
ifeq ($(HOST),Darwin)
  NPROC = sysctl -n hw.physicalcpu
else
  NPROC = nproc
endif

TARGET ?= $(HOST)
BUILD_TYPE ?= Debug
MAKE := make -f Build/Makefile -j $(shell $(NPROC))
DOCKER_EXE := docker
DOCKER ?= 1

DOCKERFILE := Build/Docker/Dockerfile
ifeq ($(TARGET),Raspi)
	DOCKERFILE := Build/Docker/Dockerfile.Raspi
endif

ENABLE_TTY =
MAKE_DOCKER = $(DOCKER_EXE) build - < $(DOCKERFILE) | tee /dev/stderr | grep "Successfully built" | cut -d ' ' -f 3
RUN = $(DOCKER_EXE) run \
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
  --rm \
  -i $(ENABLE_TTY) `$(MAKE_DOCKER)`

ifeq ($(HOST)$(TARGET),LinuxDarwin)
$(error Can't build $(TARGET) on $(HOST).)
endif
ifneq ($(TARGET),Darwin)
	ifneq (,$(wildcard /.dockerenv))
		# If we are already running inside docker
		MAKE := $(MAKE)
	else ifeq ($(DOCKER),0)
		MAKE := $(MAKE)
	else
		# Else run make inside docker
		MAKE := $(RUN) $(MAKE)
	endif
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
