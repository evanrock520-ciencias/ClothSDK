import sys
from pathlib import Path

build_dir = Path(__file__).parents[2] / "build"
python_dir = Path(__file__).parents[2] / "python"

sys.path.insert(0, str(build_dir))
sys.path.insert(0, str(python_dir))
