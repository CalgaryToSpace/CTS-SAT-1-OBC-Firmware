from pathlib import Path
import hashlib

b = Path("blob.bin").read_bytes()
print(b.hex())

print()
print(f"SHA-256 hash: {hashlib.sha256(b).hexdigest()}")
print(f"Size: {len(b)} bytes")

# 10b5034608461146034a024c7a44a047002010bd2dff02080c00000048656c6c6f20776f726c642066726f6d2074686520626c6f623a2025732100
# SHA-256 hash: 1a892a1aab2b922313a9891836a8ae17e40e20bb75986265a9fc0f48bf0b66fb
# Size: 59 bytes
