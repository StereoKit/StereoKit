# Magical, gets loaded by HunterGate.cmake
# https://hunter.readthedocs.io/en/latest/reference/user-modules/hunter_config.html?highlight=hunter_config

# get new SHA1 by  wget `https://github.com/DanielChappuis/reactphysics3d/releases/download/v0.8.0/reactphysics3d-0.8.0.tar.gz && openssl sha1 reactphysics3d-0.8.0.tar.gz`

hunter_config(
  ReactPhysics3D
    VERSION "v0.8.0"
    URL "https://github.com/DanielChappuis/reactphysics3d/releases/download/v0.8.0/reactphysics3d-0.8.0.tar.gz"
    SHA1 2a6ab45788295bb572ea05947b443cf05dfc6b2f
    CMAKE_ARGS
      CMAKE_POSITION_INDEPENDENT_CODE=ON   
)
