package com.safeway;

/**
 * This interface provides a mechanism for third party applications to extend
 * the Safeway payment system. The primary type of extension envisioned is
 * for loyalty type of programs to be notified when a payment is made
 * through the Safeway payment manager. Based on the details of the
 * payment, a loyalty program (such as United's Mileage Plus frequent flyer
 * program) may credit an account of points.
 *
 * The initial design of this class explores the mechanism by which
 * mutually untrusting apps can integrate in a limited, authenticated,
 * secure manner.
 */
public interface PaymentPlugin {

    public void notify(Payment payment);
}

