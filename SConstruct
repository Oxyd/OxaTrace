##
## Parameters
##

debug = ARGUMENTS.get('debug', 0)
verbose = ARGUMENTS.get('verbose', 0)

##
## Environments
##

defaultEnv = Environment()
defaultEnv.Append(CCFLAGS=[
  '-Wall', '-Wextra', '-std=c++11', '-pedantic', '-pthread'
])
defaultEnv.Append(LINKFLAGS=['-pthread'])
defaultEnv.Append(LIBS=['png'])
defaultEnv.Append(CPPPATH=['./eigen'])

if not verbose:
  defaultEnv.Replace(CXXCOMSTR='Compiling $TARGET')
  defaultEnv.Replace(LINKCOMSTR='Linking $TARGET')

releaseEnv = defaultEnv.Clone()
releaseEnv.Append(CCFLAGS=['-O3'])
releaseEnv.Append(CPPFLAGS=['-DNDEBUG'])

debugEnv = defaultEnv.Clone()
debugEnv.Append(CCFLAGS=['-ggdb'])

if debug:
  env = debugEnv
else:
  env = releaseEnv

##
## Rules
##

sources = Glob('src/*.cpp')
env.Program('oxatrace', sources)
Default('oxatrace')

##
## Unit tests
##

tests = ['math_test', 'shapes_test']

import subprocess
testsEnv = env.Clone()
testsEnv.VariantDir('tests-build', 'src', duplicate=0)
testsEnv.Append(CPPPATH=['./src'])
testsEnv.Append(LIBS=['gtest'])

progObjects = testsEnv.Glob('tests-build/*.cpp', strings=True)
progObjects.remove('tests-build/main.cpp')

def runTest(env, target, source):
  executable = str(source[0].abspath)
  if not subprocess.call(executable):
    file(str(target[0]), 'w').write('OK\n')

for test in tests:
  program = testsEnv.Program(
    target='src/tests/{0}'.format(test),
    source=['src/tests/{0}.cpp'.format(test), progObjects]
  )
  stamp = testsEnv.Command('src/tests/{0}.passed'.format(test),
                           program, runTest)
  Alias(test, stamp)

Alias('tests', tests)

# vim:colorcolumn=80
