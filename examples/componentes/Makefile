# Makefile of the SDK example "componentes" (design-system showcase). `make` builds build/componentes.sen
APP = componentes

SENEW_SDK ?= ../..
ifeq ($(wildcard $(SENEW_SDK)/mk/app.mk),)
  $(error SENEW_SDK=$(SENEW_SDK) is not a senew-sdk checkout (mk/app.mk missing))
endif

include $(SENEW_SDK)/mk/app.mk
