from PIL import Image


def png_to_ico(input_png, output_ico):
    img = Image.open(input_png)
    icon_sizes = [(16, 16), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)]
    img.save(output_ico, format='ICO', sizes=icon_sizes)

# Example usage

def sandbox():
    png_to_ico('icon.png', 'icon.ico')


def main():
    # parse arguments
    import argparse
    parser = argparse.ArgumentParser(description='Create a window ico file from a png image')
    parser.add_argument('input_png', help='input png image')
    parser.add_argument('output_ico', help='output ico file')
    args = parser.parse_args()

    # call create_icns
    # print(f"windows_png_icon_to_ico.py: Converting {args.input_png} to {args.output_ico}")
    png_to_ico(args.input_png, args.output_ico)


if __name__ == '__main__':
    main()
    
