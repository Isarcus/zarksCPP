add_library(${ZARKS_LIB_NAME}
    BitBuffer.cpp
    Camera.cpp
    fractals.cpp
    GaussField.cpp
    GIF.cpp
    GIFStructs.cpp
    Image.cpp
    LZWTree.cpp
    Map.cpp
    Mat3.cpp
    MemBuffer.cpp
    noise2D.cpp
    NoiseHash.cpp
    Noiser.cpp
    numerals.cpp
    RGBA.cpp
    Shape3D.cpp
    StreamKeeper.cpp
    Tessellation3D.cpp
    Triangle3D.cpp
    Vec3.cpp
)

target_link_libraries(${ZARKS_LIB_NAME} pthread)
