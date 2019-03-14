# Copyright 2019 Google LLC
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# version 2 as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.

all: FORCE build_progs

-include Makefile.local
-include $(shell find -type f -name build.mk | sort)

Q ?= @

override CPPFLAGS += -std=c99
override CPPFLAGS += -Iinclude
override CPPFLAGS += -Ithird_party
override CPPFLAGS += -D_GNU_SOURCE
override CFLAGS += -g
override CFLAGS += -O3
override CFLAGS += -Wall
override CFLAGS += -Werror

bins := $(addprefix bin/,$(progs))

build_progs: FORCE $(bins)

.%.o: %.c
	@echo -e "CC\t$@"
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

#
# The following rule is adapted from:
#   https://gnu.org/software/make/manual/html_node/Automatic-Prerequisites.html
#
# The sed command converts
#   main.o : main.c defs.h
# into
#   main.o .main.d : main.c defs.h
# so that make regenerates the prerequisites whenever .c or .h file changes.
#
.%.d: %.c
	@echo -e "GEN\t$@"
	$(Q)set -e; \
		$(RM) $@; \
		$(CC) -MM $(CPPFLAGS) $< > $@.tmp; \
		sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.tmp > $@; \
		$(RM) $@.tmp

define hidden_filenames
  $(join $(dir $(1)),$(addprefix .,$(notdir $(1))))
endef

define move_if_changed
  if test -r $@ && cmp --quiet $@.tmp $@; then \
    $(RM) $@.tmp; \
  else \
    echo -e "GEN\t$@"; \
    mv $@.tmp $@; \
  fi
endef

define prog_template
  override $(1)-obj := $$(call hidden_filenames,$$($(1)-obj))
  objs += $$($(1)-obj)
  deps += $$($(1)-obj:.o=.d)
  $(1)-cmd := $$(CC) $$(LDFLAGS) $$($(1)-obj) $$(LDLIBS) -o bin/$(1)

  bin/.$(1).cmd: FORCE
	$$(Q)echo "$$($(1)-cmd)" > $$@.tmp
	$$(Q)$$(move_if_changed)

  bin/$(1): bin/.$(1).cmd $$($(1)-obj)
	@echo -e "LD\t$$@"
	$$(Q)$$($(1)-cmd)
endef

$(foreach prog,$(progs),$(eval $(call prog_template,$(prog))))

clean: FORCE
	$(Q)$(RM) $(bins) $(objs) $(deps)

ifneq ($(MAKECMDGOALS),clean)
-include $(deps)
endif

FORCE:
