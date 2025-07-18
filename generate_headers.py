import os
import re
import subprocess
import sys

MAX_SIZE_TILES = 64000
MAX_SIZE_MAP = 4096
GRAPHICS_FOLDER = "graphics"

def run_command(command):
    """Runs a command and waits for it to complete."""
    try:
        startupinfo = None
        if sys.platform == "win32":
            startupinfo = subprocess.STARTUPINFO()
            startupinfo.dwFlags |= subprocess.STARTF_USESHOWWINDOW

        process = subprocess.run(command, check=True, capture_output=True, text=True, startupinfo=startupinfo)
        return process.stdout
    except FileNotFoundError:
        # If 'grit' is not found, try 'grit.exe' on Windows
        if sys.platform == "win32" and command[0] == "grit":
            command[0] = "grit.exe"
            try:
                process = subprocess.run(command, check=True, capture_output=True, text=True, startupinfo=startupinfo)
                return process.stdout
            except FileNotFoundError:
                print(f"Error: Could not find 'grit' or 'grit.exe'. Please ensure it's in your system's PATH.")
                sys.exit(1)
            except subprocess.CalledProcessError as e:
                print(f"Error executing command: {' '.join(command)}")
                print(e.stderr)
                sys.exit(1)
        else:
            print(f"Error: Command '{command[0]}' not found. Please ensure it is in your PATH.")
            sys.exit(1)
    except subprocess.CalledProcessError as e:
        print(f"Error executing command: {' '.join(command)}")
        print(e.stderr)
        sys.exit(1)

def extract_array_content(pattern, text):
    """Extracts the content of a C array using regex."""
    match = re.search(pattern, text, re.DOTALL)
    if match:
        # Return the content between the curly braces, stripped of whitespace.
        return match.group(1).strip()
    return None

def main():
    """Main function to generate headers."""
    print("Exporting graphics...")

    if not os.path.exists(GRAPHICS_FOLDER):
        print(f"Error: Graphics folder '{GRAPHICS_FOLDER}' not found.")
        return

    image_files = sorted([f for f in os.listdir(GRAPHICS_FOLDER) if f.endswith(('.png', '.bmp', '.gif'))])
    base_names = [os.path.splitext(f)[0] for f in image_files]

    # Run grit on all image files
    for img_file in image_files:
        input_path = os.path.join(GRAPHICS_FOLDER, img_file)
        
        command = ["grit", input_path, "-gB8", "-mRtf", "-pu16", "-ftc"]
        print(f"Running command: {' '.join(command)}")
        run_command(command)
        print(f"Exported .c/.h of {os.path.splitext(img_file)[0]}")

    print("\nAssembling graphics.h...")

    with open("graphics.h", "w") as h_file:
        # Write includes
        for name in base_names:
            h_file.write(f'#include "{name}.h"\n')
        h_file.write("\n")

        # Write NTILES define
        h_file.write(f"#define NTILES {len(base_names)}\n\n")

        # Write TilesLen array
        h_file.write("const unsigned int TilesLen[] = {\n")
        for name in base_names:
            h_file.write(f"   {name}TilesLen,\n")
        h_file.write("};\n\n")

        # Write MapsLen array
        h_file.write("const unsigned short MapsLen[] = {\n")
        for name in base_names:
            h_file.write(f"   {name}MapLen,\n")
        h_file.write("};\n\n")

        # Write PalsLen array
        h_file.write("const unsigned short PalsLen[] = {\n")
        for name in base_names:
            h_file.write(f"   {name}PalLen,\n")
        h_file.write("};\n\n")

        # --- Process generated C files for pointer arrays ---

        # Write tilesPointer array
        h_file.write(f"const unsigned int * const tilesPointer[] __attribute__((aligned(4))) = {{\n")
        for name in base_names:
            h_file.write(f"    {name}Tiles,\n")
        h_file.write("};\n\n")

        # Write mapsPointer array
        h_file.write(f"const unsigned short * const mapsPointer[] __attribute__((aligned(4))) = {{\n")
        for name in base_names:
            h_file.write(f"    {name}Map,\n")
        h_file.write("};\n\n")

        # Write palsPointer array
        h_file.write(f"const unsigned short * const palsPointer[] __attribute__((aligned(4))) = {{\n")
        for name in base_names:
            h_file.write(f"    {name}Pal,\n")
        h_file.write("};\n\n")


    print("Include file 'graphics.h' created successfully.")

if __name__ == "__main__":
    main()