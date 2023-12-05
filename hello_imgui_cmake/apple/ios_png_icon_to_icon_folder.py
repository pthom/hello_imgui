import os
import subprocess


def create_icon_folder(input_png: str, output_icon_folder: str):
    # create output_icon_folder if it doesn't exist
    if not os.path.exists(output_icon_folder):
        os.makedirs(output_icon_folder)

    # resize input_png to output_icon_folder/icon.png, size 1024x1024
    subprocess.run([
        'sips',
        '-z', '1024', '1024',
        input_png,
        '--out', os.path.join(output_icon_folder, 'icon.png')
    ])


def sandbox():
    pass


def main():
    # parse arguments
    import argparse
    parser = argparse.ArgumentParser(description='Create an ios icon folder file from a png image')
    parser.add_argument('input_png', help='input png image')
    parser.add_argument('output_icon_folder', help='output icon folder')
    args = parser.parse_args()

    # call create_icns
    # print(f"ios_png_icon_to_icon_folder.py: Converting {args.input_png} to {args.output_icon_folder}")
    create_icon_folder(args.input_png, args.output_icon_folder)


if __name__ == '__main__':
    main()
