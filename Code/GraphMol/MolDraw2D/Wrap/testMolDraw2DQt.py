from rdkit import RDConfig
import unittest
from rdkit import Chem
from rdkit.Chem import Draw, AllChem, rdDepictor
from rdkit.Chem.Draw import rdMolDraw2D

import sys
from PyQt5.Qt import *
import sip


class TestCase(unittest.TestCase):

  def setUp(self):
    pass

  def testSIPBasics(self):
    m = Chem.MolFromSmiles('c1ccccc1O')
    Draw.PrepareMolForDrawing(m)
    qimg = QImage(250, 200, QImage.Format_RGB32)
    qptr = QPainter(qimg)
    qptr._img = qimg
    p = sip.unwrapinstance(qptr)
    d2d = Draw.MolDraw2DFromQPainter_(250, 200, p)
    d2d.DrawMolecule(m)
    d2d._qptr = qptr
    qimg.save("testImageFromPyQt-1.png")
    qptr = None

  def testBasics(self):
    m = Chem.MolFromSmiles('c1ccccc1O')
    Draw.PrepareMolForDrawing(m)
    qimg = QImage(250, 200, QImage.Format_RGB32)
    qptr = QPainter(qimg)
    qptr._img = qimg

    d2d = Draw.MolDraw2DFromQPainter(qptr)
    d2d.DrawMolecule(m)
    qimg.save("testImageFromPyQt-2.png")

  def testMemory1(self):

    def testfunc():
      m = Chem.MolFromSmiles('c1ccccc1O')
      Draw.PrepareMolForDrawing(m)
      qimg = QImage(250, 200, QImage.Format_RGB32)
      qptr = QPainter(qimg)
      qptr._img = qimg
      p = sip.unwrapinstance(qptr)
      d2d = Draw.MolDraw2DFromQPainter_(250, 200, p, -1, -1)
      d2d._ptr = qptr
      raise ValueError("expected")

    with self.assertRaises(ValueError):
      testfunc()

  def testMemory2(self):

    def testfunc():
      m = Chem.MolFromSmiles('c1ccccc1O')
      Draw.PrepareMolForDrawing(m)
      qimg = QImage(250, 200, QImage.Format_RGB32)
      qptr = QPainter(qimg)
      qptr._img = qimg
      d2d = Draw.MolDraw2DFromQPainter(qptr)
      raise ValueError("expected")

    with self.assertRaises(ValueError):
      testfunc()


if __name__ == "__main__":
  app = QGuiApplication(sys.argv)
  unittest.main()
