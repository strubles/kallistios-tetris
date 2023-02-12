from PIL import Image

img = Image.open("carl_pixel.png")
img = img.convert(mode="1")
data = list(img.getdata())

width=img.size[0]
height=img.size[1]

string=""

for pixel in data:
    if pixel==0:
        string=string+"1"
    else:
        string=string+"0"

for i in range(0, int(len(string)/8)):
    if i%6==0:
        print()
    #print(string[i*8:(i*8)+8])
    print("0x{:02X}".format(int(string[i*8:(i*8)+8],2)), end=", ")
    
    
