##
## Parameters
##

mode    = ARGUMENTS.get('mode', 'release')
verbose = ARGUMENTS.get('verbose', 0)
color   = ARGUMENTS.get('color', 1)

##
## Environments
##

import os
import build_config

defaultEnv = Environment(ENV={'PATH' : os.environ['PATH']})
defaultEnv.Append(CCFLAGS=[
  '-Wall', '-Wextra', 
  '-Wno-unused-local-typedefs',  # Suppress warnings in Eigen.
  '-std=c++11', '-pedantic', '-pthread'
])
defaultEnv.Append(LINKFLAGS=['-pthread'])
defaultEnv.Append(LIBS=['m'])
defaultEnv.Append(CPPPATH=build_config.include_search_path)
defaultEnv.Append(LIBPATH=build_config.lib_search_path)

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
oxatrace = env.Program('oxatrace', sources)
Default(oxatrace)

# vim:syntax=python
