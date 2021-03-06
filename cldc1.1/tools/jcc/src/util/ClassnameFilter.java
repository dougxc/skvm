/*
 *    ClassnameFilter.java    1.7    03/01/14 SMI
 *
 * Copyright � 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package util;

import java.util.Hashtable;

/*
 * A ClassnameFilter is a pretty simple instance of a FilenameFilter.
 * It is used for processing command-line options which involve
 * lists of sets of classes. It understands a single trailing * in
 * a name to mean all the classes in a package.
 */

public
class ClassnameFilter implements java.io.FilenameFilter {

    boolean    includeAll    = false;
    boolean    conditional;

    Hashtable    includedClass   = new Hashtable(); // string class names
    Hashtable    includedPackage = new Hashtable();  // string package names

    public ClassnameFilter( boolean conditional ){
    this.conditional = conditional;
    }

    public ClassnameFilter( ){
    this(false);
    }

    public void includeClass( String className ){
    includedClass.put( className, className );
    }

    public void includePackage( String packageName ){
    if ( packageName == null || (packageName.length() == 0 ) )
        includeAll = true;
    else
        includedPackage.put( packageName, packageName );
    }

    /*
     * To include a class or package named by the given string.
     * If it ends in *, we treat it as a package name.
     * otherwise, it is a class.
     */
    public void
    includeName( String classname ){
    if ( classname.charAt( classname.length()-1 ) == '*' ){
        // this is a package name, not a class name
        // strip off trailing junk and put it on the included package
        // list.
        int pkgLength = classname.lastIndexOf( '/' );
        if ( pkgLength < 0 )
        includePackage( null );
        else
        includePackage( classname.substring(  0, pkgLength ) );
    } else {
        includeClass( classname );
    }
    }

    public boolean accept( java.io.File dir, String className ){
    if ( includedClass.get( className ) != null ){
        return true;
    } else if (includeAll) {
        return true;
    } else {
        String pkgName = className;
        int pkgLength = className.lastIndexOf('/');
        // look at immediate package, and superpackage, and ...
        while ( pkgLength > 0 ){
        pkgName = pkgName.substring( 0, pkgLength );
        if ( includedPackage.get( pkgName ) != null ){
            return true; 
        }
        pkgLength = pkgName.lastIndexOf('/');
        }
    }
    return false;
    }
}
