package com.safeway;

import java.io.*;
import java.util.*;
import javax.microedition.io.*;

/**
 * This is an overly-simple implementation of a payment manager for a
 * payment token. It simply waits for a connection from a payee's system
 * and then either effects the payment on this token or rejects it.
 *
 * At this point, the authentication and authorisation of the communication
 * with the payee system have not been implemented. They primary motivation
 * for this implementation is to explore the interaction between this
 * payment component
 * and third party components that 'register' with this app to be notified
 * of payments. Such third party components could be something like loyalty
 * point systems (i.e. frequent flyer programs).
 */
public class PaymentManager {

    /**
     * When the application is launched, it does some intial setup and then
     * just waits for a connection from a payee's system.
     *
     * In this version we emulate the communication with the payee's system
     * by manually constructing a 'request'.
     */
    public static void main(String args[]) {
        PaymentManager mgr = new PaymentManager();
        /*
         * Register a plugin class.
         */
        PaymentPluginManager.getInstance().addPlugin("com.ual.MileagePlus");
        try {
            mgr.processPayment(new Payment("safeway",100,new Date().getTime()));
        } catch (Throwable e) {
            e.printStackTrace();
        }
    }

    private void processPayment(Payment payment) {
        /*
         * Do internal processing (e.g. alter stored value on token).
         */
        System.out.println("Token is processing payment of $" +
            payment.amount + " to " + payment.payee + " on " +
            new Date(payment.date));

        /*
         * Now notify all trusted plugins of the payment.
         */
        PaymentPluginManager.getInstance().notify(payment);
    }
}
