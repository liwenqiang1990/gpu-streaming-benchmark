cmake_minimum_required(VERSION 2.6.4)

PROJECT(GPU_stream_benchmark)

#Check if OpenMP support is available and set flags.
INCLUDE(FindOpenMP)
if (OPENMP_FOUND)
  SET (HAVE_OPENMP 1)
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  MESSAGE(${OpenMP_C_FLAGS})
endif()

#Check if win32 multiprocessor compile option is available and set flags.
IF (WIN32)
  OPTION(BUILD_WITH_MP "Enables the /MP multi-processor compiler option for Visual Studio 2005 and above" ON)
   MARK_AS_ADVANCED(BUILD_WITH_MP)
   IF(BUILD_WITH_MP)
     SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
   ENDIF(BUILD_WITH_MP)
ENDIF (WIN32)

IF (${USE_32BIT})
   # Right now these flags are for gcc but they can be adapted here if necessary
   ADD_DEFINITIONS("-m32")
   SET(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-m32") 
ENDIF (${USE_32BIT})

FIND_PACKAGE(OpenGL)
FIND_PACKAGE(GLUT) 
FIND_PACKAGE(CUDA)
INCLUDE(FindGLEW.cmake)

INCLUDE_DIRECTORIES(
   ${OPENGL_INCLUDE_DIR}
   ${GLEW_INCLUDE_DIR}	
   ${GLUT_INCLUDE_DIR}
   ${CUDA_TOOLKIT_INCLUDE}
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
  cudaTestFunc.cu
  
  RenderUtility/GLSLProgram.cpp
  RenderUtility/GLBufferObject.cpp
  RenderUtility/GLframebufferObject.cpp
  RenderUtility/GLTexture.cpp
  RenderUtility/ArcBall.cpp
  RenderUtility/GL.cpp
  
   
)

set(CMAKE_BUILD_TYPE Release)

CUDA_ADD_EXECUTABLE(GPU_stream_benchmark ${RENDER_HEADERS} ${RENDER_SRC})  
TARGET_LINK_LIBRARIES(GPU_stream_benchmark
  ${OPENGL_LIBRARIES}
  ${GLEW_LIBRARIES}
  ${GLUT_LIBRARIES}
  ${CUDA_CUDA_LIBRARIES}
  ${CUDA_CUDART_LIBRARIES}
) 

