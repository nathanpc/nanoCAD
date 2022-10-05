# Define the base environment.
def_env = DefaultEnvironment()
def_env['CCFLAGS'] = '-Wall -std=c89 -Ilib/c-vector '

# Build up the engine library.
SConscript('engine/SConscript', variant_dir='build/engine', duplicate=False)

# Build up the engine library.
SConscript('tests/SConscript', variant_dir='build/tests', duplicate=False)
