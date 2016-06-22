
from PIL import Image
import numpy as np     # numpy
from collections import namedtuple
from repeat import *

r1 = Repeat(10, [1])
print(r1.r)
# >>> Repeat(10, [1])
print(r1.generate())
# >>> [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]

width = 1280
height = 720
frac_height = int(0.2*height)
frac_width = int(0.2*width)

#horizoantal wipe
rh = [Repeat(height, [Repeat(frac_width,[255]),Repeat(width-frac_width,[0])])]
mh = generate_matrix(rh,width)
to_image(mh,'horizoantal_wipe')

#horizoantal wipe
rv =  [ Repeat(frac_height, Repeat(width, [255])), Repeat(height-frac_height, Repeat(width,[0])) ]
mv = generate_matrix(rv,width)
to_image(mv,'vertical_wipe')

