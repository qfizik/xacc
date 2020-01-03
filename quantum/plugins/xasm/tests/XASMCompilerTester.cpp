/*******************************************************************************
 * Copyright (c) 2019 UT-Battelle, LLC.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompanies this
 * distribution. The Eclipse Public License is available at
 * http://www.eclipse.org/legal/epl-v10.html and the Eclipse Distribution
 *License is available at https://eclipse.org/org/documents/edl-v10.php
 *
 * Contributors:
 *   Alexander J. McCaskey - initial API and implementation
 *******************************************************************************/
#include "gtest/gtest.h"

#include "xacc.hpp"
#include "xacc_service.hpp"

#include "Circuit.hpp"
TEST(XASMCompilerTester, checkTranslate) {
  auto compiler = xacc::getCompiler("xasm");
  auto IR = compiler->compile(R"(__qpu__ void bell_test(qbit q, double t0) {
  H(q[0]);
  CX(q[0], q[1]);
  Ry(q[0], t0);
  Measure(q[0]);
  Measure(q[1]);
})");

  auto evaled = (*IR->getComposite("bell_test"))({1.1});
  auto translated = compiler->translate(evaled);
  std::cout << "HELLO:\n" << translated << "\n";
}

TEST(XASMCompilerTester, checkSimple) {

  auto compiler = xacc::getCompiler("xasm");
  auto IR =
      compiler->compile(R"(__qpu__ void bell(qbit q, double t0, double t1) {
  H(q[0]);
  CX(q[0], q[1]);
  Ry(q[0], t0);
  Ry(q[1], t1);
  U(q[1], pi, t0, t1);
  Measure(q[0]);
  Measure(q[1]);
})");

  auto bell = IR->getComposite("bell");
  EXPECT_EQ(1, IR->getComposites().size());
  EXPECT_EQ(7, bell->nInstructions());
  EXPECT_EQ(2, bell->nVariables());

  std::cout << "KERNEL\n" << bell->toString() << "\n";

  EXPECT_EQ("q", IR->getComposites()[0]->getInstruction(0)->getBufferName(0));

  IR = compiler->compile(R"([&](qbit q, double t0) {
  H(q[0]);
  CX(q[0], q[1]);
  Ry(q[0], t0);
  bell(q);
  Measure(q[0]);
  Measure(q[1]);
})");
  EXPECT_EQ(1, IR->getComposites().size());
  std::cout << "KERNEL\n" << IR->getComposites()[0]->toString() << "\n";
}

TEST(XASMCompilerTester, checkMultiRegister) {

  auto compiler = xacc::getCompiler("xasm");
  auto IR = compiler->compile(
      R"(__qpu__ void bell_multi(qbit q, qbit r, double t0, double t1) {
  H(q[0]);
  CX(q[0], q[1]);
  H(r[0]);
  CX(r[0], r[1]);
  Ry(r[1], t0);
  Rx(q[0], t1);
  Measure(q[0]);
  Measure(q[1]);
  Measure(r[0]);
  Measure(r[1]);
})");
  EXPECT_EQ(1, IR->getComposites().size());
  std::cout << "KERNEL\n" << IR->getComposites()[0]->toString() << "\n";

  EXPECT_EQ("q", IR->getComposites()[0]->getInstruction(0)->getBufferName(0));
  EXPECT_EQ("r", IR->getComposites()[0]->getInstruction(2)->getBufferName(0));
}

TEST(XASMCompilerTester, checkVectorArg) {

  auto compiler = xacc::getCompiler("xasm");
  auto IR =
      compiler->compile(R"(__qpu__ void bell22(qbit q, std::vector<double> x) {
  H(q[0]);
  CX(q[0], q[1]);
  Ry(q[0], x[0]);
  Measure(q[0]);
  Measure(q[1]);
})");
  EXPECT_EQ(1, IR->getComposites().size());
  std::cout << "KERNEL\n" << IR->getComposites()[0]->toString() << "\n";
  std::cout << "KERNEL\n"
            << IR->getComposites()[0]->operator()({2.})->toString() << "\n";
}

TEST(XASMCompilerTester, checkSimpleFor) {

  auto compiler = xacc::getCompiler("xasm");
  auto IR =
      compiler->compile(R"(__qpu__ void testFor(qbit q, std::vector<double> x) {
  for (int i = 0; i < 5; i++) {
     H(q[i]);
  }
  for (int i = 0; i < 2; i++) {
      Rz(q[i], x[i]);
  }
})");
  std::cout << "KERNEL\n" << IR->getComposites()[0]->toString() << "\n";

  IR = compiler->compile(
      R"(__qpu__ void testFor2(qbit q, std::vector<double> x) {
  for (int i = 0; i < 5; i++) {
     H(q[i]);
     Rx(q[i], x[i]);
     CX(q[0], q[i]);
  }

  for (int i = 0; i < 3; i++) {
      CX(q[i], q[i+1]);
  }
  Rz(q[3], 0.22);

  for (int i = 3; i > 0; i--) {
      CX(q[i-1],q[i]);
  }
})");
  EXPECT_EQ(1, IR->getComposites().size());
  std::cout << "KERNEL\n" << IR->getComposites()[0]->toString() << "\n";
  for (auto ii : IR->getComposites()[0]->getVariables())
    std::cout << ii << "\n";
  EXPECT_EQ(22, IR->getComposites()[0]->nInstructions());
}
TEST(XASMCompilerTester, checkHWEFor) {

  auto compiler = xacc::getCompiler("xasm");
  auto IR = compiler->compile(R"([&](qbit q, std::vector<double> x) {
    for (auto i = 0; i < 2; i++) {
        Rx(q[i],x[i]);
        Rz(q[i],x[2+i]);
    }
    CX(q[1],q[0]);
    for (auto i = 0; i < 2; i++) {
        Rx(q[i], x[i+4]);
        Rz(q[i], x[i+4+2]);
        Rx(q[i], x[i+4+4]);
    }
  })");
  EXPECT_EQ(1, IR->getComposites().size());
  std::cout << "KERNEL\n" << IR->getComposites()[0]->toString() << "\n";
  for (auto ii : IR->getComposites()[0]->getVariables())
    std::cout << ii << "\n";
  EXPECT_EQ(11, IR->getComposites()[0]->nInstructions());
}

TEST(XASMCompilerTester, checkIfStmt) {

  auto q = xacc::qalloc(2);
  q->setName("q");
  xacc::storeBuffer(q);

  auto compiler = xacc::getCompiler("xasm");
  auto IR = compiler->compile(R"([&](qbit q) {
    H(q[0]);
    Measure(q[0]);
    if (q[0]) {
        CX(q[0],q[1]);
    }
  })");
  EXPECT_EQ(1, IR->getComposites().size());
  std::cout << "KERNEL\n" << IR->getComposites()[0]->toString() << "\n";

  q->measure(0, 0);

  IR->getComposites()[0]->expand({});
  std::cout << "KERNEL\n" << IR->getComposites()[0]->toString() << "\n";

  q->measure(0, 1);

  IR->getComposites()[0]->expand({});
  std::cout << "KERNEL\n" << IR->getComposites()[0]->toString() << "\n";

  q->reset_single_measurements();
  IR->getComposites()[0]->getInstruction(2)->disable();
    std::cout << "KERNEL\n" << IR->getComposites()[0]->toString() << "\n";


  q->measure(0, 1);

  IR->getComposites()[0]->expand({});
  std::cout << "KERNEL\n" << IR->getComposites()[0]->toString() << "\n";

}

TEST(XASMCompilerTester, checkApplyAll) {
  auto compiler = xacc::getCompiler("xasm");
  auto IR = compiler->compile(R"([&](qbit q) {
  range(q, {{"gate","H"},{"nq",4}});
})");
  EXPECT_EQ(1, IR->getComposites().size());
  std::cout << "KERNEL\n" << IR->getComposites()[0]->toString() << "\n";

  IR = compiler->compile(R"([&](qbit q, double x) {
  ucc1(q, x);
})");
  EXPECT_EQ(1, IR->getComposites().size());
  std::cout << "KERNEL\n" << IR->getComposites()[0]->toString() << "\n";

  IR = compiler->compile(R"([&](qbit q, double x, double y, double z) {
  ucc3(q, x, y, z);
})");
  EXPECT_EQ(1, IR->getComposites().size());
  std::cout << "KERNEL\n" << IR->getComposites()[0]->toString() << "\n";

  IR = compiler->compile(R"([&](qbit q, std::vector<double> x) {
  ucc3(q, x[0], x[1], x[2]);
})");
  EXPECT_EQ(1, IR->getComposites().size());
  std::cout << "KERNEL\n" << IR->getComposites()[0]->toString() << "\n";

  IR = compiler->compile(R"([&](qbit q, double x) {
  exp_i_theta(q, x, {{"pauli", "X0 Y1 - X1 Y0"}});
})");
  EXPECT_EQ(1, IR->getComposites().size());
  std::cout << "KERNEL\n" << IR->getComposites()[0]->toString() << "\n";
}

int main(int argc, char **argv) {
  xacc::Initialize(argc, argv);
  xacc::set_verbose(true);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
