
tests = [
  ("python", "UnitTestMatcher.py", {}),
  ("python", "UnitTestSignature.py", {}),
  # ("python","UnitTestLazyGenerator.py",{}),
  ("python", "UnitTestUtils.py", {}),
  ("python", "UnitTestGobbi.py", {}),
  ("python", "Utils.py", {}),

  ]



longTests = [
  ]

if __name__ == '__main__':
  import sys
  from rdkit import TestRunner
  failed, tests = TestRunner.RunScript('test_list.py', 0, 1)
  sys.exit(len(failed))
