TIMEOUT ?= 240

%.h: %.ds
	cozy -t $(TIMEOUT) --allow-big-sets $^ --c++ $@ --save synthesized/$^.$(TIMEOUT).synthesized
