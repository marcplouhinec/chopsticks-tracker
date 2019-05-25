import argparse
import os

from PIL import Image
from resizeimage import resizeimage

# Parse arguments
ap = argparse.ArgumentParser()
ap.add_argument("-i", "--input", required=True, help="path to folder containing input images")
ap.add_argument("-o", "--output", required=True, help="path to folder that will contain cropped images")
args = vars(ap.parse_args())

# Find the images to crop
inputFolderPath = args["input"]
outputFolderPath = args["output"]
imageNames = os.listdir(inputFolderPath)
print(f"Crop {len(imageNames)} images...")

for imageName in imageNames:
    print(f"Crop the image: {imageName}")
    imageFile = open(inputFolderPath + "/" + imageName, "r+b")

    try:
        image = Image.open(imageFile)
        newSize = image.size[0]
        if image.size[1] < newSize:
            newSize = image.size[1]
        image = resizeimage.resize_crop(image, [newSize, newSize])
        image.save(outputFolderPath + "/" + imageName, image.format)
    except IOError:
        print(f"The file {imageName} cannot be processed.")

    imageFile.close()
