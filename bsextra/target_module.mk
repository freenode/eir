ifndef TARGETS_MODULE_MK
TARGETS_MODULE_MK=1

_BS_SUPPORTED_TARGETS += MODULE

_BS_DEFAULT_OUTPUT_MODULE = $(MODDIR)/$(1).so

define _BS_EXTRA_TARGET_SETTINGS_MODULE

$(2)_SOURCES ?= $(2).cpp

endef
#= LIBRARY_NAME_$(1)/$(2) = $(2)

# one argument: target internal name
define _BS_TARGET_RULE_MODULE

$(call add-dir,$(call dirname,$(_BS_BUILD_TARGET_$(1))))

$(_BS_BUILD_TARGET_$(1)): $(_BS_OBJECTS_$(1)) | $(call dirname,$(_BS_BUILD_TARGET_$(1)))
	rm -f $$@
	$(CXX) -shared -o $$@ \
	    $(call expand-target-variable,$(1),LDFLAGS) \
	    -L$(BUILDDIR)/$(LIBDIR) \
	    $(call expand-target-dependencies,$(1)) \
	    $(_BS_OBJECTS_$(1))

endef

endif
