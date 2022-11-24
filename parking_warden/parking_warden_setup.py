from distutils.core import setup
import py2exe # pip install matplotlib==3.2.2 else error
import matplotlib.pyplot
import matplotlib.animation
import numpy

setup(console=['parking_warden.py'])