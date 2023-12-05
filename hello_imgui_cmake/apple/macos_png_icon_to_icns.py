import os
import subprocess
import tempfile

def create_icns(input_png: str, output_icns: str):
    sizes = [16, 32, 64, 128, 256, 512, 1024]

    # Create a temp directory for the iconset
    o_tmp_iconset_folder = tempfile.TemporaryDirectory(suffix=".iconset")
    tmp_iconset_folder = o_tmp_iconset_folder.name

    # Resize image and save in the iconset folder using apple sips
    for size in sizes:
        out_image_name = os.path.join(tmp_iconset_folder, 'icon_' + str(size) + 'x' + str(size) + '.png')
        subprocess.run([
            'sips',
            '-z', str(size), str(size),
            input_png,
            '--out', out_image_name
        ],
        stdout=subprocess.DEVNULL,
        check=True
        )

    # list files in the iconset folder
    iconset_files = os.listdir(tmp_iconset_folder)

    # Create the .icns file using iconutil, redirecting stdout to /dev/null
    subprocess.run(['iconutil', '-c', 'icns', tmp_iconset_folder, '-o', output_icns],
                   stdout=subprocess.DEVNULL,  check=True)


def sandbox():
    create_icns('test.png', 'test.icns')


def main():
    # parse arguments
    import argparse
    parser = argparse.ArgumentParser(description='Create an icns file from a png image')
    parser.add_argument('input_png', help='input png image')
    parser.add_argument('output_icns', help='output icns file')
    args = parser.parse_args()

    # call create_icns
    # print(f"macos_png_icon_to_icns.py: Converting {args.input_png} to {args.output_icns}")
    create_icns(args.input_png, args.output_icns)


if __name__ == '__main__':
    main()
