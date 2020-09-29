JAVADOC = javadoc
KJavaAPI=KJavaAPI
CLDCAPI=CLDCAPI
ALLAPI=allclasses

FOOTER = '<font size="-1"><a href="mailto:kauai@eng.sun.com">Submit a bug or feature</a><br><br>Copyright (c) 2000 Sun Microsystems, Inc. All rights reserved.<br>Use is subject to license terms. Third-party software, including font technology, is copyrighted and licensed from Sun suppliers. Portions may be derived from Berkeley BSD systems, licensed from U. of CA. Sun, Sun Microsystems, the Sun logo, Java, Java 2 MicroEdition, Connected Limited Device Configuration are trademarks or registered trademarks of Sun Microsystems, Inc. in the U.S. andother countries. Federal Acquisitions:Commercial Software -Government Users Subject to Standard License Terms and Conditions.</font>'

DOCTITLE = 'CLDC Library API Specification<br><font size="-2">1.0</font>'
HEADER = '<b>CLDC</b><br><font size="-1">1.0</font>'

PDOCTITLE = 'KJAVA API<br><font size="-2">1.0</font>'
PHEADER = '<b>KJAVA</b><br><font size="-1">1.0</font>'


all: CLDCzip Palmzip CLDCpdf Palmpdf


CLDCzip:
	@echo Creating CLDCAPI.html.zip
	@rm -rf $(CLDCAPI)
	@mkdir $(CLDCAPI)
	$(JAVADOC) -sourcepath ../src \
	-d ./$(CLDCAPI) \
	-nodeprecatedlist \
	-doctitle $(DOCTITLE) \
	-header $(HEADER) \
	-windowtitle 'CLDC API' \
	-footer $(FOOTER) \
	java.lang \
	java.io \
	java.util \
	javax.microedition.io
	@rm -f $(CLDCAPI).html.zip
	@/usr/local/bin/zip -r $(CLDCAPI).html.zip $(CLDCAPI)
	
Palmzip:
	@echo Creating KJavaAPI.html.zip
	@rm -rf $(KJavaAPI)
	@mkdir $(KJavaAPI)
	$(JAVADOC) -sourcepath ../src \
	-d ./$(KJavaAPI) \
	-nodeprecatedlist \
	-doctitle $(PDOCTITLE) \
	-header $(PHEADER) \
	-windowtitle 'KJava API' \
	-footer $(FOOTER) \
	com.sun.kjava
	@rm -f $(KJavaAPI).html.zip
	@/usr/local/bin/zip -r $(KJavaAPI).html.zip $(KJavaAPI)

PACKAGES = $(shell find ../src -name "*.java"|grep -v SCCS | \
                   sed 's:^../src/::g' | \
                   sed 's:/[^/]*\.java::g' | sed 's:/:.:g'| sort | uniq)
allpackages:
	@echo Creating javadoc for all sources...
	@rm -rf all
	@mkdir all
	$(JAVADOC) -sourcepath ../src \
	-d ./all \
	-nodeprecatedlist \
	-doctitle $(DOCTITLE) \
	-header $(HEADER) \
	-windowtitle 'All Classes API' \
	-footer $(FOOTER) \
  $(PACKAGES)
	
CLDCpdf: testdocs
	@/net/piter/export/MIFDoclet/javadocMIF.sh ../src java.lang java.io java.util javax.microedition.io

Palmpdf:
	@/net/piter/export/MIFDoclet/javadocPalmMIF.sh ../src com.sun.kjava

testdocs:
	@mkdir testdocs
clean:

	@rm -rf $(KJavaAPI) $(CLDCAPI) $(KJavaAPI).html.zip $(CLDCAPI).html.zip
	@rm -rf testdocs

