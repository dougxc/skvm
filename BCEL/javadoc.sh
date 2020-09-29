find src -type d | while read f; do if [ -n "`ls $f/*.java 2>/dev/null`" ]; then echo $f; fi; done | sed 's:^src/\?::' | tr '/' '.' > .packages
javadoc -d javadocs -sourcepath src `cat .packages` src/*.java
