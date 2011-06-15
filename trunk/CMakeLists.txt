cmake_minimum_required(VERSION 2.6.4)

PROJECT(GPU_stream_benchmark)

IF (${USE_32BIT})
   # Right now these flags are for gcc but they can be adapted here if necessary
   ADD_DEFINITIONS("-m32")
   SET(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-m32") 
ENDIF (${USE_32BIT})

FIND_PACKAGE(OpenGL)
FIND_PACKAGE(glut) 
INCLUDE(FindGLEW.cmake)

INCLUDE_DIRECTORIES(
   ${OPENGL_INCLUDE_DIR}
   ${GLEW_INCLUDE_DIR}	
)

SET (RENDER_HEADERS

  
  RenderUtility/ArcBall.h
  RenderUtility/GL.h
  RenderUtility/GLBufferObject.h
  RenderUtility/GLframebufferObject.h
  RenderUtility/GLSLProgram.h
  RenderUtility/GLTexture.h
  RenderUtility/GLTextureBuffer.h
  RenderUtility/StdDefines.h
  RenderUtility/Vectors.h
  
  RenderUtility/SlotTracker3D.h
)

SET (RENDER_SRC

  main.cpp
  
  RenderUtility/GLSLProgram.cpp
  RenderUtility/GLBufferObject.cpp
  RenderUtility/GLframebufferObject.cpp
  RenderUtility/GLTexture.cpp
  RenderUtility/ArcBall.cpp
  RenderUtility/GL.cpp

 
)

ADD_EXECUTABLE(GPU_stream_benchmark ${RENDER_HEADERS} ${RENDER_SRC})  
TARGET_LINK_LIBRARIES(GPU_stream_benchmark
  ${OPENGL_LIBRARIES}
  ${GLEW_LIBRARIES}
  ${GLUT_LIBRARIES}

) 

