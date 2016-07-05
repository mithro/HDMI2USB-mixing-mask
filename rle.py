#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# vim: set ts=4 sw=4 et sts=4 ai:

import struct
from collections import namedtuple

REPEAT_OP = 1
PIXEL_OP = 2


class Pixel(int):
    """
    >>> Pixel(1).gen()
    [1]
    >>> Pixel(5).gen()
    [5]
    """

    def __new__(cls, value):
        assert value >= 0
        assert value < 256
        return int.__new__(cls, value)

    def gen(self):
        return [int(self)]

    def __repr__(self):
        return "%s(%s)" % (self.__class__.__name__, int.__repr__(self))

    def encode(self):
        """
        >>> Pixel(0).encode()
        '\\x02\\x00\\x00\\x00'
        >>> Pixel(0x10).encode()
        '\\x02\\x10\\x10\\x10'
        """
        return struct.pack("BBBB", PIXEL_OP, self, self, self)


WHITE=Pixel(255)
BLACK=Pixel(0)

_RepeatBase = namedtuple("Repeat", ["count", "contents"])
class Repeat(_RepeatBase):
    """
    >>> Repeat(1, [Pixel(1)]).gen()
    [1]
    >>> Repeat(5, [Pixel(1)]).gen()
    [1, 1, 1, 1, 1]
    >>> Repeat(1, [Pixel(1), Pixel(10)]).gen()
    [1, 10]
    >>> Repeat(3, [Pixel(1), Pixel(10)]).gen()
    [1, 10, 1, 10, 1, 10]
    """

    def __new__(cls, *args):
        if len(args) == 1:
            args = args[0]
        return _RepeatBase.__new__(cls, *args)

    def gen(self):
        contents = []
        for i in self.contents:
            contents += i.gen()
        return int(self.count) * contents

    def __repr__(self):
        return "Repeat(%r, %r)" % (self.count, self.contents)

    def encode(self):
        """
        >>> Repeat(1, []).encode()
        '\\x01\\x00\\x01\\x00'
        >>> Repeat(2, [Pixel(0)]).encode()
        '\\x01\\x01\\x02\\x00\\x02\\x00\\x00\\x00'
        >>> Repeat(3000, [Repeat(2000, [Pixel(01)]), Pixel(12), Pixel(23)]).encode()
        '\\x01\\x03\\xb8\\x0b\\x01\\x01\\xd0\\x07\\x02\\x01\\x01\\x01\\x02\\x0c\\x0c\\x0c\\x02\\x17\\x17\\x17'
        """
        data = [struct.pack("BBH", REPEAT_OP, len(self.contents), self.count)]
        for i in self.contents:
            data.append(i.encode())
        return "".join(data)


#Template([Repeat(720, lambda t: 1280-t, [BLACK]), Repeat(lambda t: t, [WHITE]))])


def TemplateEvaluate(o, t):
   """
   >>> TemplateEvaluate(Pixel(1), 1)
   Pixel(1)
   >>> TemplateEvaluate(Repeat(1, [Pixel(1)]), 1)
   Repeat(1, [Pixel(1)])
   >>> f = lambda t: Pixel(t)
   >>> TemplateEvaluate(f, 5)
   Pixel(5)
   >>> TemplateEvaluate(f, 10)
   Pixel(10)
   >>> g = Repeat(lambda t: 10-t, [Pixel(1)])
   >>> TemplateEvaluate(g, 1)
   Repeat(9, [Pixel(1)])
   >>> TemplateEvaluate(g, 9)
   Repeat(1, [Pixel(1)])
   >>> h = Repeat(lambda t: 10-t, [Repeat(lambda t: 10+t, [Pixel(1)]), Pixel(2)])
   >>> TemplateEvaluate(h, 1)
   Repeat(9, [Repeat(11, [Pixel(1)]), Pixel(2)])
   >>> TemplateEvaluate(h, 5)
   Repeat(5, [Repeat(15, [Pixel(1)]), Pixel(2)])
   """
   if callable(o):
       a = o(t)
       return a

   try:
       args = []
       for arg in o:
           args.append(TemplateEvaluate(arg, t))
       if len(args) == 1:
           return o.__class__(*args)
       return o.__class__(args)
   except TypeError as e:
       return o


if __name__ == "__main__":
    import doctest
    doctest.testmod()
