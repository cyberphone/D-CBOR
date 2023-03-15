### D-CBOR Java API
```java
import java.math.BigInteger;

import org.webpki.cbor.CBORObject;
import org.webpki.cbor.CBORInteger;
import org.webpki.cbor.CBORBigInteger;
import org.webpki.cbor.CBORDouble;
import org.webpki.cbor.CBORString;
import org.webpki.cbor.CBORMap;

import org.webpki.util.HexaDecimal;


public class dcbor {
    // Static map keys
    static CBORInteger INT_ARG         = new CBORInteger(1);
    static CBORInteger BIG_INT_ARG     = new CBORInteger(2);
    static CBORInteger BIG_INT_OPT_ARG = new CBORInteger(3);
    static CBORInteger DOUBLE_ARG      = new CBORInteger(4);
    static CBORInteger STRING_ARG      = new CBORInteger(5);


    public static void main(String[] args) {
        try {
            // CBOR encoding
            byte[] cbor = new CBORMap()
                .setObject(INT_ARG,         new CBORInteger(22))
                .setObject(BIG_INT_ARG,     new CBORBigInteger(new BigInteger("5000000000000000000000000")))
                // Programmatic creation - Automatic map sorting
                .setObject(STRING_ARG,      new CBORString("Hello dCBOR world!"))
                .setObject(BIG_INT_OPT_ARG, new CBORBigInteger(BigInteger.TEN))
                .setObject(DOUBLE_ARG,      new CBORDouble(3)).encode();
           System.out.println(HexaDecimal.encode(cbor));
/*
 CBOR encoding is https://cbor.me compatible in both directions:
 a5011602c24b0422ca8b0a00a425000000030a04f94200057248656c6c6f206443424f5220776f726c6421
*/

           // CBOR decoding
           CBORMap cborMap = CBORObject.decode(cbor).getMap();
           System.out.println(cborMap.toString());
/*
{
  1: 22,
  2: 5000000000000000000000000,
  3: 10,
  4: 3.0,
  5: "Hello dCBOR world!"
}
*/

           // CBOR map element access
           int intArg              = cborMap.getObject(INT_ARG).getInt();

           // Reading order does not matter
           String stringArg        = cborMap.getObject(STRING_ARG).getString();
           BigInteger bigIntArg    = cborMap.getObject(BIG_INT_ARG).getBigInteger();
           BigInteger bigIntOptArg = cborMap.getObject(BIG_INT_OPT_ARG).getBigInteger();

// if you comment out this line you will get error message #1
           double doubleArg        = cborMap.getObject(DOUBLE_ARG).getDouble();

// if remove comment on the next line you will get error message #2
    //       int strangerThings      = cborMap.getObject(DOUBLE_ARG).getInt();

           // Optional: check that all element were accessed
           cborMap.checkForUnread();
        } catch (Exception e) {
           System.out.println(e.getMessage());
/*
   #1 Map key 4 with argument of type=FLOATING_POINT with value=3.0 was never read
   #2 Is type: FLOATING_POINT, requested: INTEGER
*/
        }
    }
}
```
