#
#  Copyright (C) 2003  Greg Landrum and Rational Discovery LLC
#
""" defines class _DbResultSet_ for lazy interactions with Db query results

**Note**

  this uses the Python iterator interface, so you'll need python 2.2 or above.

"""
from __future__ import print_function
import sys
from rdkit.Dbase import DbInfo


class DbResultBase(object):

  def __init__(self, cursor, conn, cmd, removeDups=-1, transform=None, extras=None):
    self.cursor = cursor
    self.removeDups = removeDups
    self.transform = transform
    self.cmd = cmd
    self.conn = conn
    self.extras = extras
    self.Reset()
    self._initColumnNamesAndTypes()
    self.Reset()

  def Reset(self):
    """ implement in subclasses

    """
    try:
      if not self.extras:
        self.cursor.execute(self.cmd)
      else:
        self.cursor.execute(self.cmd, self.extras)
    except Exception:  # pragma: nocover
      sys.stderr.write('the command "%s" generated errors:\n' % (self.cmd))
      import traceback
      traceback.print_exc()

  def __iter__(self):
    self.Reset()
    return self

  def _initColumnNamesAndTypes(self):
    self.colNames = []
    self.colTypes = []
    for cName, cType in DbInfo.GetColumnInfoFromCursor(self.cursor):
      self.colNames.append(cName)
      self.colTypes.append(cType)
    self.colNames = tuple(self.colNames)
    self.colTypes = tuple(self.colTypes)

  def GetColumnNames(self):
    return self.colNames

  def GetColumnTypes(self):
    return self.colTypes

  def GetColumnNamesAndTypes(self):
    return tuple(nt for nt in zip(self.colNames, self.colTypes))


class DbResultSet(DbResultBase):
  """ Only supports forward iteration """

  def __init__(self, *args, **kwargs):
    DbResultBase.__init__(self, *args, **kwargs)

  def __iter__(self):
    self.Reset()
    seen = set()
    while True:
      r = self.cursor.fetchone()
      if not r:
        break
      if self.transform is not None:
        r = self.transform(r)
      if self.removeDups >= 0:
        v = r[self.removeDups]
        if v in seen:
          continue
        seen.add(v)
      yield r


class RandomAccessDbResultSet(DbResultBase):
  """ Supports random access """

  def __init__(self, *args, **kwargs):
    DbResultBase.__init__(self, *args, **kwargs)
    self.results = []
    self.seen = []
    self._pos = -1

  def Reset(self):
    self._pos = -1
    if self.cursor is not None:
      DbResultBase.Reset(self)

  def _finish(self):
    if self.cursor:
      r = self.cursor.fetchone()
      while r:
        if self.transform is not None:
          r = self.transform(r)
        if self.removeDups >= 0:
          v = r[self.removeDups]
          if v not in self.seen:
            self.seen.append(v)
            self.results.append(r)
        else:
          self.results.append(r)
        r = self.cursor.fetchone()
      self.cursor = None

  def __getitem__(self, idx):
    if idx < 0:
      raise IndexError("negative indices not supported")
    if self.cursor is None:
      if len(self.results):
        if idx >= len(self.results):
          raise IndexError('index %d too large (%d max)' % (idx, len(self.results)))
      else:
        raise ValueError('Invalid cursor')

    while idx >= len(self.results):
      r = None
      while r is None:
        r = self.cursor.fetchone()
        if not r:
          self.cursor = None
          raise IndexError('index %d too large (%d max)' % (idx, len(self.results)))

        if self.transform is not None:
          r = self.transform(r)
        if self.removeDups >= 0:
          v = r[self.removeDups]
          if v in self.seen:
            r = None
          else:
            self.results.append(r)
            self.seen.append(v)
        else:
          self.results.append(r)

    return self.results[idx]

  def __len__(self):
    if self.results is None:
      raise ValueError("len() not supported for noMemory Results Sets")
    self._finish()
    return len(self.results)

  def __iter__(self):
    self._finish()
    for res in self.results:
      yield res


if __name__ == '__main__':  # pragma: nocover
  from rdkit.Dbase.DbConnection import DbConnect
  conn = DbConnect('TEST.GDB')
  curs = conn.GetCursor()
  print('curs:', repr(curs))
  curs.execute('select * from ten_elements')
  resultSet = RandomAccessDbResultSet(curs)
  for i in range(12):
    try:
      val = resultSet[i]
    except IndexError:
      assert i >= 10

  print('use len')
  curs = conn.GetCursor()
  curs.execute('select * from ten_elements')
  resultSet = RandomAccessDbResultSet(curs)
  for i in range(len(resultSet)):
    val = resultSet[i]

  print('use iter')
  curs = conn.GetCursor()
  curs.execute('select * from ten_elements')
  resultSet = DbResultSet(curs)
  for thing in resultSet:
    ID, val = thing

  print('dups')
  curs = conn.GetCursor()
  curs.execute('select * from ten_elements_dups')
  resultSet = DbResultSet(curs)
  r = []
  for thing in resultSet:
    r.append(thing)
  assert len(r) == 20

  curs = conn.GetCursor()
  curs.execute('select * from ten_elements_dups')
  resultSet = DbResultSet(curs, removeDups=0)
  r = []
  for thing in resultSet:
    r.append(thing)
  assert len(r) == 10

  curs = conn.GetCursor()
  curs.execute('select * from ten_elements_dups')
  resultSet = RandomAccessDbResultSet(curs, removeDups=0)
  assert len(resultSet) == 10
  assert resultSet[0] == (0, 11)

  curs = conn.GetCursor()
  curs.execute('select * from ten_elements_dups')
  resultSet = RandomAccessDbResultSet(curs, removeDups=0)
  assert resultSet[0] == (0, 11)
  assert resultSet[1] == (2, 21)
  assert resultSet[5] == (10, 61)

  curs = conn.GetCursor()
  curs.execute('select * from ten_elements_dups')
  resultSet = RandomAccessDbResultSet(curs)
  assert resultSet[0] == (0, 11)
  assert resultSet[1] == (0, 11)
