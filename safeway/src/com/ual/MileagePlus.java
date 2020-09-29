package com.ual;

import com.safeway.PaymentPlugin;
import com.safeway.PaymentPluginManager;
import com.safeway.Payment;
import java.util.Date;

/**
 * This class represents a loyalty points manager for the United Airlines
 * Mileage Plus program. It is responsible for receiving payment
 * notifications from affiliates and adjusting some kind store loyalty
 * points value appropriately.
 */
public class MileagePlus implements PaymentPlugin {

    /**
     * The class initializer must create an instance of the class and
     * register it with the safeway plugin manager.
     */
    static {
        MileagePlus instance = new MileagePlus();
        PaymentPluginManager.getInstance().registerPluginInstance(instance);
    }

    /**
     * This is the ratio of points earned with respect to the value of a
     * payment.
     */
    private static final int POINTS_RATIO = 10;

    /**
     * Number of points earned.
     */
    private int value;

    /**
     * There is only one instance on any platform/device. This simplifies
     * access to a persistent store that this manager may use.
     */
    static private MileagePlus instance;

    /**
     * Get a handle to the instance (created during class initialization).
     */
    static MileagePlus getInstance() {
        return instance;
    }

    /**
     * Private constructor. This constructor would typically load the
     * stored value from some persistent storage.
     */
    private MileagePlus() {
        // value = load();
        value = 0;
    }


    public void notify(Payment payment) {
        /*
         * This is where all the loyalty points adjustment logic would
         * go...
         */
        int earned = payment.getAmount() / POINTS_RATIO;
        value += earned;
        // Sync with persistent store
        // save();

        System.out.print("United Mileage Plus plugin notified of " + payment +
            ": paid " + payment.getPayee() + " $" + payment.getAmount());
        System.out.println(" on " + new Date(payment.getDate()));
      
        System.out.println("\nPoints earned: "+earned);
        System.out.println("New points total: "+value);
    }
}
