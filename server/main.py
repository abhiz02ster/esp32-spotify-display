from PIL import Image
import requests
import numpy as np

def map_to_range(num, numMax, _min, _max):
    return int(float(num) / numMax * _max + _min)

def rgb_to_16_bit_color(rgb):
    # image is R5 G6 B5
    # RBG = RRRRRGGGGGGBBBBB
    # R   = 1111100000000000 = 0xF800
    # G   = 0000011111100000 = 0x07E0
    # B   = 0000000000011111 = 0x001F

    r = rgb[0]
    g = rgb[1]
    b = rgb[2]

    r = map_to_range(r, 255, 0, 31)
    g = map_to_range(g, 255, 0, 63)
    b = map_to_range(b, 255, 0, 31)

    out = r << 11 | g << 5 | b
    return out

def get_bitmap_from_url(image_url):

    # download image
    img_data = requests.get(image_url).content
    with open('./image.png', 'wb') as handler:
       handler.write(img_data)

    # open
    image = Image.open('./image.png')

    # resize
    resized_image = image.resize((200, 200))

    # convert to 1-bit monochrome with Floyd-Steinberg dithering
    try:
        dither_algo = Image.Dither.FLOYDSTEINBERG
    except AttributeError:
        dither_algo = Image.FLOYDSTEINBERG

    mono_image = resized_image.convert('1', dither=dither_algo)

    # convert to 1-bit packed byte array
    img_bytes = mono_image.tobytes()

    # Invert bits so that 1 represents Black (foreground) and 0 represents White (background)
    inverted_bytes = [b ^ 0xFF for b in img_bytes]

    return inverted_bytes