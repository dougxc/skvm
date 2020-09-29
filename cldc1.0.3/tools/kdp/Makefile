all: tools

JAVA          = java
JAVAC         = javac
JAR           = jar
JAVAFILES =  $(shell find src -name "*.java"|grep -v SCCS)
 
CLASSFILES = $(subst src,classes,$(JAVAFILES:java=class))

# $< is dependency
# $@ is target
#
$(CLASSFILES): classes/%.class : src/%.java
	@echo $< >> .filelist

eraselists:
	@rm -f .filelist 

compilefiles:
	@if [ '!' -d classes ]; then rm -rf classes; mkdir classes; fi;
	@if [ -f .filelist ]; then \
		echo $(JAVAC) `cat .filelist`; \
		$(JAVAC) -g:none -d classes -classpath classes \
	              `cat .filelist`; \
		fi

tools: eraselists $(CLASSFILES) compilefiles

classes.zip: tools
	@rm -rf classes.zip
	@$(JAR) cfM0 classes.zip -C classes .

classes.pdb: classes.zip
	@echo Creating classes.pdb
	@CLASSPATH=../tools/classes java palm.database.ZipPDB \
	     classes.pdb ClassesDB kJav Data classes.zip
clean:
	rm -rf *.zip *.pdb
	rm -rf .filelist
	rm -rf classes
	rm -rf *~ */*~ */*/*~
	rm -rf *# */*# */*/*#

