ARCHS = arm64
TARGET = iphone:clang:14.5:9.0
INSTALL_TARGET_PROCESSES = Jubeat

XCODE_SDK := $(shell xcrun --sdk iphoneos --show-sdk-path 2>/dev/null)
ifneq ($(XCODE_SDK),)
SYSROOT ?= $(XCODE_SDK)
ISYSROOT ?= $(XCODE_SDK)
endif

include $(THEOS)/makefiles/common.mk

# Xcode 16+ no longer accepts the legacy v1 Substrate TBD bundled by older
# Theos checkouts. Prefer the equivalent local v4 stub when it is present.
ifneq ($(XCODE_SDK),)
_THEOS_INTERNAL_SEARCHPATHS := $(CURDIR)/vendor $(_THEOS_INTERNAL_SEARCHPATHS)
endif

TWEAK_NAME = JubeatDisplayFix
JubeatDisplayFix_FILES = Tweak.xm
JubeatDisplayFix_CFLAGS = -fobjc-arc -Wno-module-import-in-extern-c
JubeatDisplayFix_FRAMEWORKS = UIKit

include $(THEOS_MAKE_PATH)/tweak.mk
