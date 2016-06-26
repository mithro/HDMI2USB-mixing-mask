
from PIL import Image
import numpy as np     # numpy
from collections import namedtuple

repeat = namedtuple('repeat', 'count pixel')

class Repeat:

	def __init__(self,count1,pixel2):
		self.r = repeat(count = count1, pixel = pixel2)

#	def count(self):
#		return self[0]
#	def pixel(self):
#		return self[1]

	def generate(self):
		"""  returns a list repeated pixel_list count number of times """		
		mylist = []
		pixel_list = []
		count = self.r.count
		pixel = self.r.pixel

		if(type(pixel)==list):
			if(type(pixel[0])==int): 
				pixel_list = pixel 
			else:
				for j in range(len(pixel)):
					pixel_list = pixel_list + pixel[j].generate()
		else:
			pixel_list = pixel.generate()	

		for i in range(count):
			for j in range(len(pixel_list)): 
				mylist.append(pixel_list[j])	
		return mylist	

def to_matrix(list_1D,row_length):
	"""  Converts a 1D list to 2D list as per the row_length information """		
	return [list_1D[i:i+row_length] for i in range(0,len(list_1D),row_length)]
		
def to_image(list_2D, file_name):
	"""  Converts a 2D list with values in [0-255] to image data type and saves as .png file"""		
	list_2D = np.uint8(list_2D)
	im = Image.fromarray(np.asarray(list_2D))
	im.save('%s.png' % file_name)
	return im

def generate_matrix(repeat_obj,width):
	""" takes a list of Repeat object and returns the corresponding 2D matrix"""
	list_1D = []
	for i in repeat_obj:
		list_1D = list_1D + i.generate()

	return to_matrix(list_1D, width)
