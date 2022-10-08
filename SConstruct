import os

# Define the base environment.
def_env = DefaultEnvironment()
def_env['ENV'] = {'PATH': os.environ['PATH']}
def_env['CCFLAGS'] = '-Wall -std=c89 -Ilib/c-vector '

# Check if we are building with debug symbols.
debug = ARGUMENTS.get('debug', 0)
if int(debug):
   def_env['CCFLAGS'] += '-g3 -DDEBUG '

# Build up the engine and graphics library.
SConscript('engine/SConscript', variant_dir='build/engine', duplicate=False)
SConscript('graphics/SConscript', variant_dir='build/graphics', duplicate=False)

# Build up the engine library.
SConscript('tests/SConscript', variant_dir='build/tests', duplicate=False)
