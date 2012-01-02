/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation,
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file    testRot3.cpp
 * @brief   Unit tests for Rot3Q class
 * @author  Richard Roberts
 */

#include <CppUnitLite/TestHarness.h>
#include <gtsam/base/Testable.h>
#include <boost/math/constants/constants.hpp>
#include <gtsam/base/numericalDerivative.h>
#include <gtsam/base/lieProxies.h>
#include <gtsam/geometry/Point3.h>
#include <gtsam/geometry/Rot3.h>
#include <gtsam/nonlinear/NonlinearFactorGraph.h>
#include <gtsam/nonlinear/Values.h>
#include <gtsam/nonlinear/NonlinearOptimization.h>
#include <gtsam/slam/BetweenFactor.h>
#include <gtsam/slam/PriorFactor.h>

using namespace gtsam;

typedef TypedSymbol<Rot3, 'r'> Key;
typedef Values<Key> Rot3Values;
typedef PriorFactor<Rot3Values, Key> Prior;
typedef BetweenFactor<Rot3Values, Key> Between;
typedef NonlinearFactorGraph<Rot3Values> Graph;

/* ************************************************************************* */
TEST(Rot3, optimize) {

  // Optimize a circle
  Rot3Values truth;
  Rot3Values initial;
  Graph fg;
  fg.add(Prior(0, Rot3(), sharedSigma(3, 0.01)));
  for(int j=0; j<6; ++j) {
    truth.insert(j, Rot3::Rz(M_PI/3.0 * double(j)));
    initial.insert(j, Rot3::Rz(M_PI/3.0 * double(j) + 0.1 * double(j%2)));
    fg.add(Between(j, (j+1)%6, Rot3::Rz(M_PI/3.0), sharedSigma(3, 0.01)));
  }

  NonlinearOptimizationParameters params;
  Rot3Values final = optimize(fg, initial, params);

  EXPECT(assert_equal(truth, final, 1e-5));
}

/* ************************************************************************* */
int main() {
  TestResult tr;
  return TestRegistry::runAllTests(tr);
}
/* ************************************************************************* */

