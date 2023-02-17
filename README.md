# D-CBOR - Deterministic Serialization for CBOR

### Abstract
_D-CBOR is a CBOR counterpart to the Distinguished Encoding Rules (DER) 
of ASN.1. D-CBOR builds on the preferred serialization options outlined 
in RFC8949, but with another target: secure cryptographic operations that
does not require CBOR encoded data to be hashed etc. to be wrapped in 
byte-strings or similar in order to return a repeatable result
Although not the primary goal, by constraining serialization options,
testing becomes easier, which in turn should foster interoperability._

### Example
The following example illustrates what can be accomplished with D-CBOR:

From https://datatracker.ietf.org/doc/html/rfc8392#appendix-A.3 we take an example of a CBOR Web Token (CWT):
```cbor
18([h'a10126', {
    4: h'4173796d6d65747269634543445341323536'
}, h'a70175636f61703a2f2f61732e6578616d706c652e636f6d02656572696b77037818636f61703a2f2f6c696768742e6578616d706c652e636f6d041a5612aeb0051a5610d9f0061a5610d9f007420b71', h'5427c1ff28d23fbad1f29c4c7c6a555e601d6fa29f9179bc3d7438bacaca5acd08c8d4d4f96131680c429a01f85951ecee743a52b9b63632c57209120e1c9e30'])
```

The very same object using an enveloped signature:
```cbor
18({
    1: "coap://as.example.com",
    2: "erikw",
    3: "coap://light.example.com",
    4: 1444064944,
    5: 1443944944,
    6: 1443944944,
    7: h'0b71',
    / Signature container /
    -1: {
      / alg = ES256 /
      1: -7,
      / kid /
      3: h'4173796d6d65747269634543445341323536',
      / signature value which encompasses all data including the top level tag /
      6: h'ffb944aef83cac4fcfab3c5a8e5ecc1bf725a75d0e8014687ae7a7b611c1c238e145e43bbea9cb229d360a85224fb439c2cc25cda922e5e54965f6c9626bad59'
    }
})
```
Note: the signature scheme utilized above is an example and not a part D-CBOR.
What D-CBOR brings to the table, is the ability keeping data _unwrapped_,
even in the case decoded data is _reencoded_.
This is by no means new; X.509 certificates have thanks to ASN.1 DER,
been encoded in a similar fashion since their inception in the early 90ties.

### Constraints
To be supplied

### Internet Draft
To be supplied

### FAQ
__Q:__ Is D-CBOR a redefined CBOR?<br>
__A:__ No, D-CBOR is 100% compatible with
[RFC8949](https://www.rfc-editor.org/rfc/rfc8949.html)
but imposes certain restrictions in order to achieve the stated goals.

__Q:__ Is D-CBOR usable with constrained devices?<br>
__A:__ Absolutely! Take a look at 
[d-cbor-4-constrained-devices.md](d-cbor-4-constrained-devices.md)
for more details.

__Q:__ If you don't know in advance how many elements there will be in
an array, you can't use D-CBOR, right?<br>
__A:__ This scenario is addressed by
"[fixups](d-cbor-4-constrained-devices.md#handling-indefinite-length-data)"
which in _most_ cases can replace indefinite-length encoding.

__Q:__ Home does _reencoding_ come into the picture?<br>
__A:__ If you use cryptographic constructs like enveloped signatures
or certain variants of authenticated encryption, data must stay
in its original form between the decoding and reencoding
steps mandated by such schemes.
D-CBOR enables this without wrapping data in `bstr` or `base64url`.

__Q:__ Does D-CBOR affect decoders?<br>
__A:__ Yes, advanced decoder/encoder designs typically use a _single
class to represent a specific data type_.  Deterministic serialization requires
instances of such objects to serialize identically,
regardless if they are created programmatically or are the
result of a decoding process.
All in the name of symmetry 😁

### Author
A. Rundgren
