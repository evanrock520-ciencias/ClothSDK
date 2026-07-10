import pathlib
import sys


def main():
    path = pathlib.Path("CMakeLists.txt")
    if not path.is_file():
        print(
            f"Error: CMakeLists.txt not found in {pathlib.Path.cwd()}",
            file=sys.stderr,
        )
        sys.exit(1)

    print(f"Patching CMakeLists.txt in {pathlib.Path.cwd()}...")
    try:
        content = path.read_text(encoding="utf-8")
        lines = content.splitlines()
        new_lines = []
        replaced = False
        for line in lines:
            if "CMP0042" in line and "OLD" in line:
                new_line = line.replace("OLD", "NEW")
                new_lines.append(new_line)
                replaced = True
                print(f"  Replaced: {line.strip()} -> {new_line.strip()}")
            else:
                new_lines.append(line)

        if replaced:
            path.write_text("\n".join(new_lines) + "\n", encoding="utf-8")
            print("Successfully patched.")
        else:
            print("CMP0042 OLD not found, no patch needed.")
    except Exception as e:
        print(f"Error during patching: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
