file(GLOB CMSIS_DSP_SRC 
  drivers/libraries/cmsis/dsp/Source/BasicMathFunctions
  drivers/libraries/cmsis/dsp/Source/BayesFunctions/BayesFunctions.c
  drivers/libraries/cmsis/dsp/Source/CommonTables/CommonTables.c
  drivers/libraries/cmsis/dsp/Source/ComplexMathFunctions/ComplexMathFunctions.c
  drivers/libraries/cmsis/dsp/Source/ControllerFunctions/ControllerFunctions.c
  drivers/libraries/cmsis/dsp/Source/DistanceFunctions/DistanceFunctions.c
  drivers/libraries/cmsis/dsp/Source/FastMathFunctions/FastMathFunctions.c
  drivers/libraries/cmsis/dsp/Source/FilteringFunctions/FilteringFunctions.c
  drivers/libraries/cmsis/dsp/Source/InterpolationFunctions/InterpolationFunctions.c
  drivers/libraries/cmsis/dsp/Source/MatrixFunctions/MatrixFunctions.c
  drivers/libraries/cmsis/dsp/Source/QuaternionMathFunctions/QuaternionMathFunctions.c
  drivers/libraries/cmsis/dsp/Source/StatisticsFunctions/StatisticsFunctions.c
  drivers/libraries/cmsis/dsp/Source/SupportFunctions/SupportFunctions.c
  drivers/libraries/cmsis/dsp/Source/SVMFunctions/SVMFunctions.c
  drivers/libraries/cmsis/dsp/Source/TransformFunctions/TransformFunctions.c
)

set(CMSIS_DSP_INC
  drivers/libraries/cmsis/dsp/include
  drivers/libraries/cmsis/dsp/PrivateInclude
)