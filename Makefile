include scripts/config.mak

# Disable default Make rules
.SUFFIXES:

OUTPUT_DIRECTORY = outputs
object_for = $(addprefix $(OUTPUT_DIRECTORY)/,$(addsuffix .o,$(basename $(1))))

default: $(OUTPUT_DIRECTORY)/epsilon.$(EXE)

# Define a standard rule helper
# If passed a last parameter value of with_local_version, we also define an
# extra rule that can build source files within the $(OUTPUT_DIRECTORY). This is
# useful for rules that can be applied for intermediate objects (for example,
# when going .png -> .cpp -> .o).
define rule_for
$(addprefix $$(OUTPUT_DIRECTORY)/,$(strip $(2))): $(strip $(3)) | $$$$(@D)/.
	@ echo "$(shell printf "%-8s" $(strip $(1)))$$(@:$$(OUTPUT_DIRECTORY)/%=%)"
	$(Q) $(4)
ifeq ($(strip $(5)),with_local_version)
$(addprefix $$(OUTPUT_DIRECTORY)/,$(strip $(2))): $(addprefix $$(OUTPUT_DIRECTORY)/,$(strip $(3)))
	@ echo "$(shell printf "%-8s" $(strip $(1)))$$(@:$$(OUTPUT_DIRECTORY)/%=%)"
	$(Q) $(4)
endif
endef

.PHONY: info
info:
	@echo "EPSILON_VERSION = $(EPSILON_VERSION)"
	@echo "EPSILON_ONBOARDING_APP = $(EPSILON_ONBOARDING_APP)"
	@echo "EPSILON_BOOT_PROMPT = $(EPSILON_BOOT_PROMPT)"
	@echo "EPSILON_APPS = $(EPSILON_APPS)"
	@echo "EPSILON_I18N = $(EPSILON_I18N)"

# Since we're building out-of-tree, we need to make sure the output directories
# are created, otherwise the receipes will fail (e.g. gcc will fail to create
# "output/foo/bar.o" because the directory "output/foo" doesn't exist).
# We need to mark those directories as precious, otherwise Make will try to get
# rid of them upon completion (and fail, since those folders won't be empty).
.PRECIOUS: $(OUTPUT_DIRECTORY)/. $(OUTPUT_DIRECTORY)%/.
$(OUTPUT_DIRECTORY)/. $(OUTPUT_DIRECTORY)%/.:
	$(Q) mkdir -p $(dir $@)

# To make objects dependent on their directory, we need a second expansion
.SECONDEXPANSION:

# Each sub-Makefile can either add sources to the $(src) variable or define a
# new executable target. The $(src) variable lists the sources that will be
# built and linked to every executable being generated.

ifeq ($(USE_LIBA),0)
include liba/Makefile.bridge
else
SFLAGS += -ffreestanding -nostdinc -nostdlib
include liba/Makefile
include libaxx/Makefile
endif
include ion/Makefile
include kandinsky/Makefile
include poincare/Makefile
include python/Makefile
include escher/Makefile
# Executable Makefiles
include apps/Makefile
include scripts/struct_layout/Makefile
include scripts/scenario/Makefile
include quiz/Makefile # Quiz needs to be included at the end

objs = $(call object_for,$(src))

# Load source-based dependencies
# Compilers can generate Makefiles that states the dependencies of a given
# objet to other source and headers. This serve no purpose for a clean build,
# but allows correct yet optimal incremental builds.
-include $(objs:.o=.d)

.SECONDARY: $(objs)
$(OUTPUT_DIRECTORY)/epsilon.$(EXE): $(objs)
$(OUTPUT_DIRECTORY)/test.$(EXE): $(objs)

# Define standard compilation rules

$(eval $(call rule_for, \
  AS, %.o, %.s, \
  $$(CC) $$(SFLAGS) -c $$< -o $$@ \
))

$(eval $(call rule_for, \
  CC, %.o, %.c, \
  $$(CC) $$(SFLAGS) $$(CFLAGS) -c $$< -o $$@, \
  with_local_version \
))

$(eval $(call rule_for, \
  CXX, %.o, %.cpp, \
  $$(CC) $$(SFLAGS) $$(CXXFLAGS) -c $$< -o $$@, \
  with_local_version \
))

$(eval $(call rule_for, \
  LD, %.$$(EXE), , \
  $$(LD) $$^ $$(LDFLAGS) -o $$@ \
))

.PHONY: clean
clean:
	@echo "CLEAN"
	$(Q) rm -rf $(OUTPUT_DIRECTORY)

.PHONY: cowsay_%
cowsay_%:
	@echo " -------"
	@echo "| $(*F) |"
	@echo " -------"
	@echo "        \\   ^__^"
	@echo "         \\  (oo)\\_______"
	@echo "            (__)\\       )\\/\\"
	@echo "                ||----w |"
	@echo "                ||     ||"

.PHONY: clena
clena: cowsay_CLENA clean

-include scripts/targets.$(PLATFORM).mak
