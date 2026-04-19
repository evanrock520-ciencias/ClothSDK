from setuptools import setup, find_packages

setup(
    name="tissu",
    version="1.0.0",
    packages=find_packages(where="python"),
    package_dir={"": "python"},
    install_requires=[],
    python_requires=">=3.8",
)
