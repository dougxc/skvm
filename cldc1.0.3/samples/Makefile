TOP=..
include $(TOP)/build/Makefile.inc

ifeq ($(DEBUG),true)
DEBUGFLAG=
else
DEBUGFLAG=":none"
endif

PREVERIFY = ../tools/preverifier/build/$(PLATFORM)/preverify
JAVA      = java
JAVAC     = javac
JAR       = jar

ifeq ($(PLATFORM), palm)
all: tools samples
else
all: tools
endif

JAVAFILES =  $(shell find src -name "*.java"|grep -v SCCS)

CLASSFILES = $(subst src,classes,$(JAVAFILES:java=class))

MAKEPALMAPP = -classpath ../tools/palm/classes.zip palm.database.MakePalmApp
PALMAPPFLAGS = -v -version '"$(VERSION)"' -bootclasspath ../api/classes \
               -classpath classes
MAKEPALMAPPCMD = $(JAVA) $(MAKEPALMAPP) $(PALMAPPFLAGS)

# $< is dependency
# $@ is target

$(CLASSFILES): classes/%.class : src/%.java
	@echo $< >> .filelist

eraselists:
	@rm -f .filelist

compilefiles:
	@if [ '!' -d tmpclasses ]; then rm -rf tmpclasses; mkdir tmpclasses; fi;
	@if [ -f .filelist ]; then \
		echo $(JAVAC) $(EXTRAJAVACFLAGS) -g$(DEBUGFLAG) -d tmpclasses \
		      -classpath tmpclasses:../api/classes \
	              -bootclasspath ../api/classes `cat .filelist`; \
		$(JAVAC) $(EXTRAJAVACFLAGS) -g$(DEBUGFLAG) -d tmpclasses \
                      -classpath tmpclasses:../api/classes \
	              -bootclasspath ../api/classes \
                      `cat .filelist` || exit 1; \
		echo $(PREVERIFY) -d classes -classpath ../api/classes \
                      tmpclasses; \
		$(PREVERIFY) -d classes -classpath ../api/classes \
                      tmpclasses || exit 1; \
		fi

tools: eraselists $(CLASSFILES) compilefiles

samples:
	$(MAKEPALMAPPCMD) -icon icons/dragon.bmp dragon.Dragon
	$(MAKEPALMAPPCMD) -icon icons/manyballs.bmp manyBalls.ManyBalls
	$(MAKEPALMAPPCMD) -icon icons/pong.bmp pong.Pong
	$(MAKEPALMAPPCMD) -icon icons/scribble.bmp scribble.Scribble
	$(MAKEPALMAPPCMD) -icon icons/cruiser.bmp -name Cruiser \
                starCruiser.StarCruiser
	$(MAKEPALMAPPCMD) -icon icons/threed.bmp -name Sun3D \
                threeDLogo.ThreeDLogo
	$(MAKEPALMAPPCMD) -icon icons/missile.bmp missiles.Missiles \
                missiles.Bomb missiles.GameBitmap missiles.GamePiece \
                missiles.Launcher missiles.Missile missiles.Missiles2 \
                missiles.ShipLg missiles.ShipMd missiles.ShipSm missiles.ShipXL
	$(MAKEPALMAPPCMD) -icon icons/dots.bmp dots.DotGame dots.PenTaps \
                dots.TopDown dots.Random dots.Average
	$(MAKEPALMAPPCMD) -icon icons/default.bmp ui.UITest
	$(MAKEPALMAPPCMD) -icon icons/queens.bmp eightQueens.EightQueens
	$(MAKEPALMAPPCMD) -icon icons/default.bmp hanoiTowers.HanoiTowers

classes.zip: tools
	@rm -rf classes.zip
	@$(JAR) cfM0 classes.zip -C classes .
	@$(JAR) ufM0 classes.zip -C ../api/classes .

classes.pdb: classes.zip
	CLASSPATH=../tools/classes java palm.database.ZipPDB \
	    classes.pdb ClassesDB kJav Data classes.zip
clean:
	rm -rf *.zip *.pdb *.prc
	rm -rf .filelist
	rm -rf classes
	rm -rf tmpclasses
	rm -rf *~ */*~ */*/*~
	rm -rf *# */*# */*/*#

