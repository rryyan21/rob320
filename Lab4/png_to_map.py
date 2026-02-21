from PIL import Image
import numpy as np
import sys

def png_to_map(input_file, output_file):
    try:
        img = Image.open(input_file)
        
        if img is None:
            raise ValueError("Could not open or find the image")
        
        img = img.convert('L')  # Convert to grayscale
        img_array = np.array(img)
        height, width = img_array.shape
        
        with open(output_file, 'w') as f:
            f.write(f'{width} {height}\n')
            img_array[img_array > 0] = 1
            np.savetxt(f, img_array, fmt='%d', delimiter=' ')
                
        print(f"Converted {input_file} to {output_file}")
    except Exception as e:
        print(f"An error occurred: {e}")

def main(argv):
    if len(argv) != 2:
        print("Usage: python png_to_map.py <input_png_file>")
        sys.exit(1)
    
    input_file = argv[1]
    output_file = input_file.split('/')[-1].replace('.png', '.map')
    
    png_to_map(input_file, output_file)

if __name__ == "__main__":
    main(sys.argv)