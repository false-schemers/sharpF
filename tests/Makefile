.PHONY: all clean realclean

tests = tests-a \
        tests-b \
        tests-c \
        tests-d \
        tests-e \
        tests-f \
        tests-g \
        tests-h \
        tests-i \
        tests-l \
        tests-m

.PHONY: $(tests:%=run-%)

helpers = ../lib/libs.sf \
          helpers.sf

hroots = $(helpers:%.sf=%)

exe =
CC = clang -Wno-parentheses-equality
SFC = ../fixpoint/sfc

roots = $(tests) $(hroots)

gens = $(roots:%=%.c) $(tests:%=%$(exe))

all: $(tests:%=run-%)

clean:
	$(RM) $(gens)

realclean: clean

$(tests:%=run-%): run-%: %
	./$^

$(tests): %: %.c $(hroots:%=%.c)
	$(CC) -o $@ $^

$(tests:%=%.c): %.c : %.sf $(hroots:%=%.c)
	$(SFC) -v $<

helpers.c: helpers.sf ../lib/libs.c
	$(SFC) -v $<

../lib/libs.c: ../lib/libs.sf
	$(SFC) -v $<
