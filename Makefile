.PHONY: all cli gui test clean

all: cli

cli:
	$(MAKE) -C cSyn

gui:
	$(MAKE) -C cSyn/GUI

test:
	$(MAKE) -C cSyn test
	$(MAKE) -C unittest test

clean:
	$(MAKE) -C cSyn clean
	$(MAKE) -C cSyn/GUI clean
	$(MAKE) -C unittest clean
