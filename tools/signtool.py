#!/usr/bin/env python3

import sys, os.path
from nacl.signing import SigningKey, VerifyKey
from nacl.encoding import HexEncoder
from nacl.exceptions import BadSignatureError, CryptoError
from nacl import pwhash, secret, utils

def usage():
    n = os.path.basename(sys.argv[0])
    print("%s sign <private key file> <file to sign> --- sign a file" % n, file=sys.stderr)
    print("%s keygen [-p] <private key file> --- generate private key and save it, output public key" % n, file=sys.stderr)
    print("%s pubkey <private key file> --- read private key and write public key to stdout" % n, file=sys.stderr)
    print("%s verify <pubkey> <signature> <file to verify> --- " % n, file=sys.stderr)
    exit(1)


def sbox_from_pw(salt_in=None):
    sys.stderr.write('enter password: ')
    pw = input().strip().encode()
    
    if not salt_in: salt = utils.random(pwhash.argon2i.SALTBYTES)
    else: salt = salt_in
    
    print('hashing key...', file=sys.stderr)
    key = pwhash.argon2i.kdf(secret.SecretBox.KEY_SIZE, pw,
        salt,
        opslimit=pwhash.argon2i.OPSLIMIT_SENSITIVE, memlimit=pwhash.argon2i.MEMLIMIT_SENSITIVE)
        
    sbox = secret.SecretBox(key)
    if salt_in:
        return sbox
    return sbox, salt

def maybe_decrypt(privkeyhex):
    # private keys are 32 byte, (64 in hexadecimal)
    # encrypted private keys are longer
    if len(privkeyhex) == 64:
        # private key not encrypted
        return privkeyhex
    # data contains the password salt and the encrypted private key
    data = HexEncoder.decode(privkeyhex)
    # get secret box with salt and user input
    sbox = sbox_from_pw(data[:pwhash.argon2i.SALTBYTES])
    # remove salt from data
    data = data[pwhash.argon2i.SALTBYTES:]
    try:
        data = sbox.decrypt(data)
    except CryptoError as e:
        # wrong password
        print(str(e), file=sys.stderr)
        exit(1)
    return HexEncoder().encode(data)

def sign():
    try:
        privatekeyfile, file_to_sign = sys.argv[2:]
    except:
        usage()

    key = SigningKey(maybe_decrypt(open(privatekeyfile).read().strip()), HexEncoder)
    if file_to_sign == "-":
        data = sys.stdin.read()
    else:
        data = open(file_to_sign, 'rb').read()

    print(key.sign(data, encoder=HexEncoder).signature.decode())

def keygen():
    encrypt = False
    try:
        if len(sys.argv) > 3:
            if sys.argv[2] == "-p":
                encrypt = True
            else:
                usage()
            privatekeyfile = sys.argv[3]
        else:
            privatekeyfile = sys.argv[2]
    except:
        usage()
    
    # generate new key
    key = SigningKey.generate()
    
    if encrypt:
        sbox, salt = sbox_from_pw()
        key_out = HexEncoder.encode(salt + sbox.encrypt(key.encode())).decode()
    else:
        key_out = key.encode(HexEncoder).decode()
    
    open(privatekeyfile, 'x').write(key_out)
    
    # print public key
    print(key.verify_key.encode(HexEncoder).decode())


def pubkey():
    privatekey = None
    if len(sys.argv) > 2:
        try:
            privatekey = open(sys.argv[2]).readline().strip()
        except:
            usage()
    if privatekey == None or privatekey == '-':
        privatekey = sys.stdin.readline().strip()

    key = SigningKey(maybe_decrypt(privatekey), HexEncoder)
    print(key.verify_key.encode(HexEncoder).decode())


def verify():
    try:
        pubkey, signature, file_to_verify = sys.argv[2:]
    except:
        usage()

    if file_to_verify == "-":
        data = sys.stdin.read()
    else:
        data = open(file_to_verify, 'rb').read()
    
    key = VerifyKey(pubkey, HexEncoder)
    
    try:
        key.verify(data, HexEncoder.decode(signature))
        print("OK")
    except BadSignatureError:
        print("INVALID")
        exit(1)
        

try:
    subcmd = sys.argv[1]
except:
    usage()

if subcmd == "sign": sign()
elif subcmd == "keygen": keygen()
elif subcmd == "pubkey": pubkey()
elif subcmd == "verify": verify()
else: usage()
