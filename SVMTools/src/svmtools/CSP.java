package svmtools;

import java.security.Signature;
import java.security.SignatureException;
import java.security.Key;
import java.security.PrivateKey;
import java.security.PublicKey;
import java.security.InvalidKeyException;
import java.io.DataInputStream;
import java.io.IOException;

/**
 * This class represents the interface to the Cryptographic Service
 * Provider (CSP) for the Wobulator. The CSP provides the Wobulator the
 * functionality of a digital signature algorithm and a key encoding format.
 */
public abstract class CSP {

    /**
     * This is the identifier of the CSP.
     */
    private final String identifier;

    /**
     * The Signature object used for signing/verifying digital signatures.
     */
    private final Signature signature;

    /**
     * Creates a CSP for a given CSP identifier.
     *
     * @param identifier  the identifier of the CSP implementation
     * @param signature   the signing/verification algorithm
     */
    protected CSP (String identifier, Signature signature) {
        this.identifier = identifier;
        this.signature = signature;
    }

    /**
     * Generates a CSP object that implements a specific signature algorithm and provides specific encodings.
     *
     * @param clazz  identifies a specific implementation class.
     * @return an instance of the class denoted by <code>clazz</code> or null if no such class is available.
     */
    public static CSP getInstance(String clazz) {
        try {
            Class c = Class.forName(clazz);
            CSP csp = (CSP)c.newInstance();
            return csp;
        } catch (ClassNotFoundException cnfe) {
        } catch (InstantiationException ie) {
        } catch (IllegalAccessException iae) {
        }
        return null;
    }

    /**
     * Gets the identifier of this CSP implementation.
     *
     * @return the identifier of this CSP implementation
     */
    public final String getIdentifier() {
        return identifier;
    }

    /**
     * Confirms that a given type of public key is supported.
     *
     * @param key       the key to test
     * @param isPublic  specifies if <code>key</code> is a public key
     * @return true if the type of <code>key</code> is supported
     */
    public abstract boolean isKeyTypeSupported(Key key, boolean isPublic);

    /**
     * Decodes a signature from a DataInputStream.
     *
     * @param in   the DataInputStream holding an encoded signature
     * @return the signature's bytes
     * @throws CSPException if the reading of the input stream fails or the signature is improperly encoded
     */
    public abstract byte[] decodeSignature(DataInputStream in)
        throws CSPException;

    /**
     * Computes the signature on a given array of bytes with a given private key and return the result.
     *
     * @param data  the content to be signed
     * @param key   the private key to be used for signing
     * @throws CSPException if the key type does not match the key type supported by this CSP for signing
     */
    public final byte[] sign(byte[] data, PrivateKey key) throws CSPException {
        if (key == null)
            throw new CSPException("signing key cannot be null");
        if (data == null)
            throw new CSPException("cannot sign null message");
        try {
            signature.initSign(key);
            signature.update(data);
            return signature.sign();
        }
        catch (InvalidKeyException ike) {
            throw new CSPException ("invalid key");
        }
        catch (SignatureException se) {
            // this should *never* happen
        }
        return null;
    }

    /**
     * Verifies a signature.
     *
     * @param data   the data that was signed to produce the signature
     * @param sig    the signature to be verified
     * @param key    the public key used for verification
     * @return true if verification succeeds, false otherwise
     * @throws CSPException if the key type does not match the key type supported by this CSP for verification
     */
    public final boolean verify(byte[] data, byte[] sig, PublicKey key) throws CSPException {
        try {
            signature.initVerify(key);
            signature.update(data);
            return signature.verify(sig);
        }
        catch (InvalidKeyException ike) {
            throw new CSPException ("invalid key");
        }
        catch (SignatureException se) {
            throw new CSPException(se.getMessage());
        }
    }

    /**
     * Encodes a public key in the format used to store it in a trusted classfile.
     *
     * @param key   the key to be encoded
     * @throws CSPException if the key type does not match the key type supported by this CSP
     */
    public abstract byte[] encodePublicKey(PublicKey key)
        throws CSPException;

    /**
     * Decodes a public key from a trusted classfile.
     *
     * @param key   the key as encoded in a trusted classfile
     * @return the PublicKey object reconstructed from the trusted classfile data
     * @throws CSPException if the key type does not match the key type supported by this CSP
     */
    public abstract PublicKey decodePublicKey(byte key[])
        throws CSPException;

    /**
     * Encodes a private key in the format used to store it in a trusted classfile.
     *
     * @param key   the key to be encoded
     * @throws CSPException if the key type does not match the key type supported by this CSP
     */
    public abstract byte[] encodePrivateKey(PrivateKey key)
        throws CSPException;

    /**
     * Decodes a private key from a trusted classfile.
     *
     * @param key   the key as encoded in a trusted classfile
     * @return the PrivateKey object reconstructed from the trusted classfile data
     * @throws CSPException if the key type does not match the key type supported by this CSP
     */
    public abstract PrivateKey decodePrivateKey(byte key[])
        throws CSPException;
}
