import os
Import("env")

# include toolchain paths
env.Replace(COMPILATIONDB_INCLUDE_TOOLCHAIN=True)
print("don't run random code on ur computers kids...")

# override compilation DB path
env.Replace(COMPILATIONDB_PATH="compile_commands.json")
