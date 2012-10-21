##
## Parameters
##

mode    = ARGUMENTS.get('mode', 'release')
verbose = ARGUMENTS.get('verbose', 0)
color   = ARGUMENTS.get('color', 1)

##
## Environments
##

defaultEnv = Environment()
defaultEnv.Append(CCFLAGS=[
  '-Wall', '-Wextra', '-std=c++11', '-pedantic', '-pthread'
])
defaultEnv.Append(LINKFLAGS=['-pthread'])
defaultEnv.Append(LIBS=['png', 'm'])
defaultEnv.Append(CPPPATH=['./eigen'])

if not verbose:
  defaultEnv.Replace(ARCOMSTR='Archiving $TARGET')
  defaultEnv.Replace(RANLIBCOMSTR='Indexing $TARGET')
  defaultEnv.Replace(CXXCOMSTR='Compiling $TARGET')
  defaultEnv.Replace(LINKCOMSTR='Linking $TARGET')

releaseEnv = defaultEnv.Clone()
releaseEnv.Append(CCFLAGS=['-O3'])
releaseEnv.Append(CPPFLAGS=['-DNDEBUG'])

debugEnv = defaultEnv.Clone()
debugEnv.Append(CCFLAGS=['-ggdb'])
debugEnv.Append(CPPDEFINES=['_GLIBCXX_DEBUG', '_GLIBCXX_DEBUG_PEDANTIC'])

profileEnv = releaseEnv.Clone()
profileEnv.Append(CCFLAGS=['-ggdb'])

environments = {
  'release': releaseEnv,
  'debug': debugEnv,
  'profile': profileEnv
}

try:
  env = environments[mode]
except KeyError:
  import sys
  print >> sys.stderr, 'Unknown mode {0}'.format(mode)
  Exit(1)

##
## Rules
##

sources = Glob('src/*.cpp')
env.Program('oxatrace', sources)
Default('oxatrace')

##
## Unit tests
##

# Need to build Google Test so that the unit tests can link with it.
gtestEnv = env.Clone()
gtestEnv.Append(CPPPATH=['gtest', 'gtest/include'])

gtest = gtestEnv.Library(source='gtest/src/gtest-all.cc',
                         target='gtest/gtest')

tests = ['math_test', 'solids_test', 'scene_test', 'image_test', 'color_test']

import subprocess
testsEnv = env.Clone()
testsEnv.VariantDir('tests-build', 'src', duplicate=0)
testsEnv.Append(CPPPATH=['src', 'gtest/include'])
testsEnv.Append(LIBS=[gtest])

progObjects = testsEnv.Glob('tests-build/*.cpp', strings=True)
progObjects.remove('tests-build/main.cpp')

def _runTest(env, target, source):
  executable = str(source[0].abspath)
  cmdline = [executable]
  if color != '0': cmdline.append('--gtest_color=yes')
  if not subprocess.call(cmdline):
    file(str(target[0]), 'w').write('OK\n')
runTest = Action(_runTest, 'Running $SOURCE')

for test in tests:
  program = testsEnv.Program(
    target='src/tests/{0}'.format(test),
    source=['src/tests/{0}.cpp'.format(test), progObjects]
  )
  stamp = testsEnv.Command('src/tests/{0}.passed'.format(test),
                           program, runTest)
  Alias(test, stamp)

Alias('tests', tests)

# vim:syntax=python
