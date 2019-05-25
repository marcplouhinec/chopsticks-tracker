import argparse
import os

from PIL import Image
from resizeimage import resizeimage

# Parse arguments
ap = argparse.ArgumentParser()
ap.add_argument("-i", "--input", required=True, help="path to folder containing input images")
ap.add_argument("-o", "--output", required=True, help="path to folder that will contain resized images")
ap.add_argument("-w", "--width", required=True, help="target width")
ap.add_argument("-k", "--height", required=True, help="target height")
args = vars(ap.parse_args())

# Find the images to crop
inputFolderPath = args["input"]
outputFolderPath = args["output"]
width = int(args["width"])
height = int(args["height"])
imageNames = os.listdir(inputFolderPath)
print(f"Resize {len(imageNames)} images...")

for imageName in imageNames:
    print(f"Resize the image: {imageName}")
    imageFile = open(inputFolderPath + "/" + imageName, "r+b")

    try:
        image = Image.open(imageFile)
        image = resizeimage.resize_contain(image, [width, height])
        image.save(outputFolderPath + "/" + imageName, image.format)
    except IOError:
        print(f"The file {imageName} cannot be processed.")

    imageFile.close()
