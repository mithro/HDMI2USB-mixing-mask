
from PIL import Image
import numpy as np     # numpy
from collections import namedtuple

_RepeatBase = namedtuple('repeat', ['count','contents'])

class Repeat(_RepeatBase):
	"""Repeat class inherits from the _RepeatBase namedtuple"""

	def generate(self):
		return self.count*self.contents.gen()


class Pixel (object):
	"""Pixel class which has a generate function which returns itself"""
	def __init__(self,pixel_val):
		self.pixel_val = pixel_val

	def gen(self) :
		mylist = []

		if type(self.pixel_val) == int :
			mylist.append(self.pixel_val)

		elif type(self.pixel_val) == list :
			for i in self.pixel_val:
				mylist = mylist + i.generate()
		else :
			mylist.append(self.pixel_val.generate())	

		return mylist

BLACK = Pixel(1)
WHITE = Pixel(255)

r1 = Repeat(1,BLACK)
r2 = Repeat(3,WHITE)

p2 = Pixel([r1,r2])
r = Repeat(2, p2)
print(r.generate())

