from pathlib import Path
import hashlib

b = Path("blob.bin").read_bytes()
print(b.hex())

print()
print(f"SHA-256 hash: {hashlib.sha256(b).hexdigest()}")
print(f"Size: {len(b)} bytes")

# 10b5034608461146034a024c7a44a047002010bd25d802080c00000048656c6c6f20776f726c642066726f6d2074686520626c6f623a2025732100
