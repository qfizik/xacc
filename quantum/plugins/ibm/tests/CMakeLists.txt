# *******************************************************************************
# Copyright (c) 2019 UT-Battelle, LLC.
# All rights reserved. This program and the accompanying materials
# are made available under the terms of the Eclipse Public License v1.0
# and Eclipse Distribution License v.10 which accompany this distribution.
# The Eclipse Public License is available at http://www.eclipse.org/legal/epl-v10.html
# and the Eclipse Distribution License is available at
# https://eclipse.org/org/documents/edl-v10.php
#
# Contributors:
#   Alexander J. McCaskey - initial API and implementation
# *******************************************************************************/
include_directories(${CMAKE_SOURCE_DIR}/quantum/plugins/ibm/accelerator)
include_directories(${CMAKE_SOURCE_DIR}/quantum/plugins/ibm/compiler)
include_directories(${CMAKE_SOURCE_DIR}/quantum/gate/utils)
include_directories(${CMAKE_BINARY_DIR})

add_xacc_test(LocalIBMAccelerator)
target_link_libraries(LocalIBMAcceleratorTester xacc-quantum-gate)

add_xacc_test(OQASMCompiler)
target_link_libraries(OQASMCompilerTester xacc-quantum-gate)

add_xacc_test(QObjectCompiler)
target_link_libraries(QObjectCompilerTester CppMicroServices)

#add_xacc_test(IBMAccelerator)
#target_link_libraries(IBMAcceleratorTester xacc-ibm)