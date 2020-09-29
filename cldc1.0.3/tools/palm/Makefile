all: classes.zip

JAVA          = java
JAVAC         = javac
CLASSDIR      = classes/palm/database
JAR           = jar

JAVAFILES =  $(wildcard src/palm/database/*.java)
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
		echo javac `cat .filelist`; \
		javac -d classes -classpath classes `cat .filelist`; \
	fi

tools: eraselists $(CLASSFILES) compilefiles 

$(CLASSDIR)/Wrapper.prc: src/palm/database/Wrapper.prc
	@cp $< $@
$(CLASSDIR)/DefaultTiny.bmp: ../../samples/icons/DefaultTiny.bmp
	@cp $< $@

classes.zip: tools $(CLASSFILES) $(CLASSDIR)/Wrapper.prc $(CLASSDIR)/DefaultTiny.bmp
	@rm -f classes.zip
	@$(JAR) cfM0 classes.zip -C classes .

%.prc: classes.zip ../../samples/classes/%.class
	@echo ... $@
	@JAVA_COMPILER=NONE CLASSPATH=classes.zip \
	        java palm.database.MakePalmApp \
		-v -v -classpath ../../samples/classes \
	        -bootclasspath ../../api/classes.zip $(basename $@)

clean:
	rm -rf .filelist
	rm -rf classes
	rm -rf *.o */*.o */*/*.o
	rm -rf *~ */*~ */*/*~
	rm -rf *# */*# */*/*#
	rm -f classes.zip
