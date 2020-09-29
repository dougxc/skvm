
package com.safeway;

import javax.microedition.cbs.*;
import java.util.Hashtable;
import java.util.Enumeration;

/**
 * This class manages the loading of plugins that are notified of payments.
 * The set of plugin classes that are searched for is also maintained by
 * this class and in a real deployment it would include some kind of
 * communication channel to update this set.
 *
 * It is a singleton class.
 */
public final class PaymentPluginManager {

    /**
     * The singleton instance.
     */
    static private PaymentPluginManager instance;

    /**
     * Return the instance (creating it if necessary).
     * Note that this method has public scope so that the plugin classes
     * can get a handle to the singleton instance for the purpose of
     * registerting an instance of themselves. The method they use for this
     * (i.e. registerPluginInstance) is the only other public scoped method
     * of this class.
     */
    static public PaymentPluginManager getInstance() {
        if (instance == null) {
            instance = new PaymentPluginManager();
        }
        return instance;
    }

    /**
     * This is the set of registered plugin instances.
     */
    private Hashtable plugins;

    /**
     * Add a plugin class to the set of registered plugins. Note that this
     * class is package-private which means only the Safeway app can update
     * the plugin set.
     * @param name The name of the plugin class.
     * @return whether or not the class was found and an instance
     * successfully registered. If this is false, then a diagnostic
     * message will have be sent to System.err
     */
    boolean addPlugin(String name) {
        if (plugins.contains(name)) {
            System.err.println("Plugin "+name+" already registered");
            return false;
        }

        try {
            Class.forName(name);
            // The corresponding plugin slot should have been filled.
            if (plugins.get(name) == null) {
                System.err.println("Warning: " + name +
                    " did not register an instance.");
                return false;
            }
            return true;
        }
        catch (ClassNotFoundException cnfe) {
            System.err.println("Class "+name+" not found");
        }
        catch (IllegalSubclassError ise) {
                System.err.println("Subclass verification failed for " +
                    name + ": " + ise.getMessage());
        }
        catch (LinkageError le) {
            /*
             * There was a problem with the classfile but this should
             * not prevent the Safeway component from continuing.
             * Note that this handles the case where the class
             * initializer causes an exception.
             */
            System.out.println("A LinkageError occurred while loading "+
                name + ": " +le.getMessage());
        }
        return false;
    }
    
    /**
     * Remove a given plugin.
     */
    void removePlugin(String name) {
        plugins.remove(name);
    }

    /**
     * Hide constructor.
     */
    private PaymentPluginManager() {
        plugins = new Hashtable();
    }

    /**
     * This static method must be called by the static initialiser of a
     * plugin class. It registers the singleton instance of that class that
     * is to be notified of payment(s). This avoids the Manager from
     * requiring instantiation privileges for each plugin class.
     */
    public void registerPluginInstance(PaymentPlugin plugin) {
        String name = plugin.getClass().getName();
        /*
         * No class should attempt to register more than one plugin.
         * Successive registrations for the same class are quietly ignored.
         */
        if (!plugins.contains(name)) {
            plugins.put(name,plugin);
            System.out.println("registered " + name);
        }
    }

    /**
     * Notify each plugin of a payment. This method implements a specific
     * notification policy.
     * Note that this method has package scope.
     */
    void notify(Payment payment) {
        for (Enumeration e = plugins.elements(); e.hasMoreElements();) {
            PaymentPlugin plugin = (PaymentPlugin)e.nextElement();
            String name = plugin.getClass().getName();
            try {
                System.out.println("Notifying "+name);
                plugin.notify(payment);
            } catch (Throwable t) {
                // Protect Safeway app from any problems in plugin
                System.err.println("Exception while notifying "+name+":");
                t.printStackTrace();
            }
        }
    }           
}

