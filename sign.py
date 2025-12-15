import argparse
from Crypto.PublicKey import RSA
from Crypto.Signature import pss
from Crypto.Hash import SHA3_256 as SHA3_256_Hash

def load_private_key(path):
    with open(path, 'rb') as f:
        return RSA.import_key(f.read())

def sign_file(file_path, private_key):
    h = SHA3_256_Hash.new()
    with open(file_path, 'rb') as f:
        for chunk in iter(lambda: f.read(4096), b''):
            h.update(chunk)
    signer = pss.new(private_key)
    signature = signer.sign(h)
    return signature

def main():
    parser = argparse.ArgumentParser(description="Digitally sign a file using RSA private key and SHA3-256.")
    parser.add_argument("private_key", help="Private key PEM file.")
    parser.add_argument("input_file", help="File to sign.")
    parser.add_argument("--output_signature", default="signature.bin", help="Output signature file.")
    args = parser.parse_args()

    key = load_private_key(args.private_key)
    signature = sign_file(args.input_file, key)
    with open(args.output_signature, 'wb') as f:
        f.write(signature)
    print(f"Signature saved to {args.output_signature}")

if __name__ == "__main__":
    main()
