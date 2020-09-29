package com.safeway;

import java.util.Date;

/**
 * This class represents the basic information describing a payment.
 * All fields of this object are immutable once the object is constructed.
 */
public class Payment {

    /**
     * These are the details of a payment. Note that these fields have
     * package level protection (i.e. they are not 'private', 'protected'
     * or 'public'). This allows efficient access to details of a payment
     * from other classes in the com.safeway package while forcing classes
     * from other packages to use accessor methods (which may optionally
     * add extra security).
     */
    String payee;
    int    amount;
    long   date;  // 'long' representation for immutability
    
    /**
     * Construct a payment.
     * @param payee The identifier of the entity (system/person/payment
     * station) to which the payment is being made.
     * @param amount The amount of the payment.
     * @param date The timestamp of the payment.
     */
    public Payment(String payee, int amount, long date)
        throws IllegalArgumentException
    {
        if (payee == null || payee.length() == 0)
            throw new IllegalArgumentException("payee ID must not be empty");
        if (amount <= 0)
            throw new IllegalArgumentException("amount must be greater than 0");
        if (date > new Date().getTime())
            throw new IllegalArgumentException("date cannot be in the future");
        this.payee  = payee;
        this.amount = amount;
        this.date   = date;
    }

    /**
     * These are the accessor methods that must be used by classes external
     * to the com.safeway package to retrieve the internal details of a
     * payment.
     */
    public String getPayee()   { return payee;  }
    public int    getAmount()  { return amount; }
    public long   getDate()    { return date;   } 
}
