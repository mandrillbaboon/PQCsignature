import argparse
from Crypto.PublicKey import RSA
from Crypto.Signature import pss
from Crypto.Hash import SHA3_256

def load_public_key(path):
    with open(path, 'rb') as f:
        return RSA.import_key(f.read())

def verify_signature(file_path, signature_path, public_key):
    h = SHA3_256.new() 
    with open(file_path, 'rb') as f:
        for chunk in iter(lambda: f.read(4096), b''):
            h.update(chunk)

    with open(signature_path, 'rb') as f:
        signature = f.read()

    verifier = pss.new(public_key)
    try:
        verifier.verify(h, signature)
        return True
    except (ValueError, TypeError):
        return False

def main():
    parser = argparse.ArgumentParser(description="Verify digital signature using RSA public key and SHA3-256.")
    parser.add_argument("public_key", help="Public key PEM file.")
    parser.add_argument("signature_file", help="Signature file.")
    parser.add_argument("original_file", help="Original file to verify.")
    args = parser.parse_args()

    key = load_public_key(args.public_key)
    valid = verify_signature(args.original_file, args.signature_file, key)
    if valid:
        print("SIGNATURE VERIFIED: Valid signature.")
    else:
        print("SIGNATURE VERIFICATION FAILED: Invalid signature.")

if __name__ == "__main__":
    main()
