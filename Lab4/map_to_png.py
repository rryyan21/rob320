from PIL import Image
import numpy as np
import sys

def value_to_color(value):
    if value == 1:
        return (255, 255, 255)
    r = (value * 37) % 256
    g = (value * 57) % 256
    b = (value * 97) % 256
    return (r, g, b)

def load_map(filename):
    with open(filename, 'r') as file:
        _ = file.readline() # Skip the first line
        map_data = np.loadtxt(file, dtype=int)
    return map_data

def create_image(map_data):
    height, width = map_data.shape
    image = Image.new('RGB', (width, height))

    unique_values = np.unique(map_data)
    for value in unique_values:
        color = value_to_color(value)
        for y in range(height):
            for x in range(width):
                if map_data[y, x] == value:
                    image.putpixel((x, y), color)

    return image

def display_image(image):
    image.show()

def save_image(image, filename):
    image.save(filename)

def main(argv):
    if len(argv) != 2:
        print("Usage: python show_map.py <map_file>")
        sys.exit(1)
    filename = argv[1]
    map_data = load_map(filename)
    image = create_image(map_data)
    image_filename = filename.replace('.map', '.png')
    save_image(image, image_filename)
    print("Image saved as {}".format(image_filename))

if __name__ == "__main__":
    main(sys.argv)