TOP=..
include $(TOP)/build/Makefile.inc

ifeq ($(DEBUG),true)
DEBUGFLAG=
else
DEBUGFLAG=":none"
endif

JAVAC     = javac
PREVERIFY = ../tools/preverifier/build/$(PLATFORM)/preverify
JAR = jar

all: tools

JAVAFILES =  $(shell find src -name "*.java"|grep -v SCCS|grep -v j2se)

CLASSFILES = $(subst src,classes,$(JAVAFILES:java=class))

# $< is dependency
# $@ is target
#
$(CLASSFILES): classes/%.class : src/%.java
	@echo $< >> .filelist

eraselists:
	@rm -f .filelist

compilefiles:
	@if [ '!' -d tmpclasses ]; then rm -rf tmpclasses; mkdir tmpclasses; fi;
	@if [ -f .filelist ]; then \
		echo $(JAVAC) $(EXTRAJAVACFLAGS) -g$(DEBUGFLAG) -d tmpclasses -target 1.3 -source 1.3 -bootclasspath classes \
		      -classpath classes `cat .filelist`; \
		$(JAVAC) $(EXTRAJAVACFLAGS) -g$(DEBUGFLAG) -d tmpclasses -target 1.3 -source 1.3 -bootclasspath classes \
		      -classpath classes `cat .filelist` || exit 1; \
		echo $(PREVERIFY) -d classes tmpclasses; \
		$(PREVERIFY) -d classes tmpclasses || exit 1; \
		fi

tools: eraselists $(CLASSFILES) compilefiles

classes.zip: tools
	@rm -rf classes.zip
	@$(JAR) cfM0 classes.zip -C classes .

clean:
	rm -rf *.zip
	rm -rf .filelist
	rm -rf classes
	rm -rf tmpclasses
	rm -rf *~ */*~ */*/*~
	rm -rf *# */*# */*/*#

