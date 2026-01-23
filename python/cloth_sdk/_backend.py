import sys
import os
from pathlib import Path

def load():
    current_path = Path(__file__).resolve()
    build_path = current_path.parent.parent.parent / "build"

    if (build_path.exists()):
        sys.path.append(str(build_path))
        print(f"[Python] Search path added: {build_path}")
    else:
        print(f"[Error] Build directory not found at {build_path}")
        sys.exit(0)