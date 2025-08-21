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

    print("\nAssembling graphics.h and graphics.c...")

    # Create graphics.h (header file with declarations only)
    with open("graphics.h", "w") as h_file:
        # Write ifndef
        h_file.write('#ifndef GRAPHICS_H\n')
        h_file.write('#define GRAPHICS_H\n\n')
        
        # Write includes
        for name in base_names:
            h_file.write(f'#include "{name}.h"\n')
        h_file.write("\n")

        # Write NTILES define
        h_file.write(f"#define NTILES {len(base_names)}\n\n")

        # Write extern declarations for arrays
        h_file.write("// Array length declarations\n")
        h_file.write("extern const unsigned int TilesLen[];\n")
        h_file.write("extern const unsigned short MapsLen[];\n")
        h_file.write("extern const unsigned short PalsLen[];\n\n")

        h_file.write("// Pointer array declarations\n")
        h_file.write("extern const unsigned int * const tilesPointer[] __attribute__((aligned(4)));\n")
        h_file.write("extern const unsigned short * const mapsPointer[] __attribute__((aligned(4)));\n")
        h_file.write("extern const unsigned short * const palsPointer[] __attribute__((aligned(4)));\n\n")
        
        # Close ifndef
        h_file.write('#endif\n\n')

    # Create graphics.c (source file with definitions)
    with open("graphics.c", "w") as c_file:
        # Write includes
        c_file.write('#include "graphics.h"\n\n')

        # Write TilesLen array
        c_file.write("const unsigned int TilesLen[] = {\n")
        for name in base_names:
            c_file.write(f"   {name}TilesLen,\n")
        c_file.write("};\n\n")

        # Write MapsLen array
        c_file.write("const unsigned short MapsLen[] = {\n")
        for name in base_names:
            c_file.write(f"   {name}MapLen,\n")
        c_file.write("};\n\n")

        # Write PalsLen array
        c_file.write("const unsigned short PalsLen[] = {\n")
        for name in base_names:
            c_file.write(f"   {name}PalLen,\n")
        c_file.write("};\n\n")

        # Write tilesPointer array
        c_file.write(f"const unsigned int * const tilesPointer[] __attribute__((aligned(4))) = {{\n")
        for name in base_names:
            c_file.write(f"    {name}Tiles,\n")
        c_file.write("};\n\n")

        # Write mapsPointer array
        c_file.write(f"const unsigned short * const mapsPointer[] __attribute__((aligned(4))) = {{\n")
        for name in base_names:
            c_file.write(f"    {name}Map,\n")
        c_file.write("};\n\n")

        # Write palsPointer array
        c_file.write(f"const unsigned short * const palsPointer[] __attribute__((aligned(4))) = {{\n")
        for name in base_names:
            c_file.write(f"    {name}Pal,\n")
        c_file.write("};\n\n")

    print("Include file 'graphics.h' and source file 'graphics.c' created successfully.")

if __name__ == "__main__":
    main()