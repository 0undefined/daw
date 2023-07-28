# Cmake targets

The preferred way to include daw in your CMake project is to use `FetchContent`:
```CMake
FetchContent_Declare(daw
  GIT_REPOSITORY https://github.com/0undefined/daw.git
  GIT_TAG        latest
)
```
