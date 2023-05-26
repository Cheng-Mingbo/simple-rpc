# recursively find all .cpp files in this directory and assign them to the SOURCES variable
file(GLOB_RECURSE SOURCES "*.cpp")

# specify the target sources
target_sources(rpc PRIVATE ${SOURCES})
