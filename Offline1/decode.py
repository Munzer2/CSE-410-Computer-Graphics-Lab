from sympy.ntheory.residue_ntheory import discrete_log
from binascii import unhexlify

p, g = 14060974547413669781, 5
A, B = 1830246353465130139, 9221743904784204759

# 1) recover Alice's private exponent a
a = discrete_log(p, A, g)

# 2) compute shared secret
s = pow(B, a, p)

# 3) decrypt
cipher = unhexlify("575d5853435b0651000e0f075118575b01011e0601575b19590b50561e56010652010a5a5f565154534c33")
key    = str(s).encode()
plain  = bytes(cipher[i] ^ key[i % len(key)] for i in range(len(cipher)))

print("flag =", plain.decode())


# p: 14060974547413669781, g: 5
# Alice's Public Key: 1830246353465130139
# Bob's Public Key: 9221743904784204759

# Ciphertext: 575d5853435b0651000e0f075118575b01011e0601575b19590b50561e56010652010a5a5f565154534c33
