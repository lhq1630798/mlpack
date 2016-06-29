/**
 * @file akfn_test.cpp
 *
 * Tests for KFN (k-furthest-neighbors) with different values of epsilon.
 */
#include <mlpack/core.hpp>
#include <mlpack/methods/neighbor_search/neighbor_search.hpp>
#include <mlpack/core/tree/cover_tree.hpp>
#include <boost/test/unit_test.hpp>
#include "test_tools.hpp"

using namespace mlpack;
using namespace mlpack::neighbor;
using namespace mlpack::tree;
using namespace mlpack::metric;
using namespace mlpack::bound;

BOOST_AUTO_TEST_SUITE(AKFNTest);

/**
 * Test the dual-tree furthest-neighbors method with different values for
 * epsilon. This uses both a query and reference dataset.
 *
 * Errors are produced if the results are not according to relative error.
 */
BOOST_AUTO_TEST_CASE(AproxVsExact1)
{
  arma::mat dataset;

  if (!data::Load("test_data_3_1000.csv", dataset))
    BOOST_FAIL("Cannot load test dataset test_data_3_1000.csv!");

  KFN exact(dataset);
  arma::Mat<size_t> neighborsExact;
  arma::mat distancesExact;
  exact.Search(dataset, 15, neighborsExact, distancesExact);

  for (size_t c = 0; c < 4; c++)
  {
    KFN* akfn;
    double epsilon;

    switch (c)
    {
      case 0: // Use the dual-tree method with e=0.02.
        epsilon = 0.02;
        break;
      case 1: // Use the dual-tree method with e=0.05.
        epsilon = 0.05;
        break;
      case 2: // Use the dual-tree method with e=0.10.
        epsilon = 0.10;
        break;
      case 3: // Use the dual-tree method with e=0.20.
        epsilon = 0.20;
        break;
    }

    // Now perform the actual calculation.
    akfn = new KFN(dataset, false, false, epsilon);
    arma::Mat<size_t> neighborsAprox;
    arma::mat distancesAprox;
    akfn->Search(dataset, 15, neighborsAprox, distancesAprox);

    for (size_t i = 0; i < neighborsAprox.n_elem; i++)
      REQUIRE_RELATIVE_ERR(distancesAprox(i), distancesExact(i), epsilon);

    // Clean the memory.
    delete akfn;
  }
}

/**
 * Test the dual-tree furthest-neighbors method with the exact method.  This
 * uses only a reference dataset.
 *
 * Errors are produced if the results are not according to relative error.
 */
BOOST_AUTO_TEST_CASE(AproxVsExact2)
{
  arma::mat dataset;

  if (!data::Load("test_data_3_1000.csv", dataset))
    BOOST_FAIL("Cannot load test dataset test_data_3_1000.csv!");

  KFN exact(dataset);
  arma::Mat<size_t> neighborsExact;
  arma::mat distancesExact;
  exact.Search(15, neighborsExact, distancesExact);

  KFN akfn(dataset, false, false, 0.05);
  arma::Mat<size_t> neighborsAprox;
  arma::mat distancesAprox;
  akfn.Search(15, neighborsAprox, distancesAprox);

  for (size_t i = 0; i < neighborsAprox.n_elem; i++)
    REQUIRE_RELATIVE_ERR(distancesAprox[i], distancesExact[i], 0.05);
}

/**
 * Test the single-tree furthest-neighbors method with the exact method.  This
 * uses only a reference dataset.
 *
 * Errors are produced if the results are not according to relative error.
 */
BOOST_AUTO_TEST_CASE(SingleTreeVsExact)
{
  arma::mat dataset;

  if (!data::Load("test_data_3_1000.csv", dataset))
    BOOST_FAIL("Cannot load test dataset test_data_3_1000.csv!");

  KFN exact(dataset);
  arma::Mat<size_t> neighborsExact;
  arma::mat distancesExact;
  exact.Search(15, neighborsExact, distancesExact);

  KFN akfn(dataset, false, true, 0.05);
  arma::Mat<size_t> neighborsAprox;
  arma::mat distancesAprox;
  akfn.Search(15, neighborsAprox, distancesAprox);

  for (size_t i = 0; i < neighborsAprox.n_elem; i++)
    REQUIRE_RELATIVE_ERR(distancesAprox[i], distancesExact[i], 0.05);
}

/**
 * Test the cover tree single-tree furthest-neighbors method against the exact
 * method.  This uses only a random reference dataset.
 *
 * Errors are produced if the results are not according to relative error.
 */
BOOST_AUTO_TEST_CASE(SingleCoverTreeTest)
{
  arma::mat dataset;
  dataset.randu(75, 1000); // 75 dimensional, 1000 points.

  KFN exact(dataset);
  arma::Mat<size_t> neighborsExact;
  arma::mat distancesExact;
  exact.Search(dataset, 15, neighborsExact, distancesExact);

  StandardCoverTree<EuclideanDistance, NeighborSearchStat<FurthestNeighborSort>,
      arma::mat> tree(dataset);

  NeighborSearch<FurthestNeighborSort, LMetric<2>, arma::mat, StandardCoverTree>
      coverTreeSearch(&tree, true, 0.05);

  arma::Mat<size_t> neighborsCoverTree;
  arma::mat distancesCoverTree;
  coverTreeSearch.Search(dataset, 15, neighborsCoverTree, distancesCoverTree);

  for (size_t i = 0; i < neighborsCoverTree.n_elem; ++i)
    REQUIRE_RELATIVE_ERR(distancesCoverTree[i], distancesExact[i], 0.05);
}

/**
 * Test the cover tree dual-tree furthest neighbors method against the exact
 * method.
 *
 * Errors are produced if the results are not according to relative error.
 */
BOOST_AUTO_TEST_CASE(DualCoverTreeTest)
{
  arma::mat dataset;
  data::Load("test_data_3_1000.csv", dataset);

  KFN exact(dataset);
  arma::Mat<size_t> neighborsExact;
  arma::mat distancesExact;
  exact.Search(dataset, 15, neighborsExact, distancesExact);

  StandardCoverTree<EuclideanDistance, NeighborSearchStat<FurthestNeighborSort>,
      arma::mat> referenceTree(dataset);

  NeighborSearch<FurthestNeighborSort, LMetric<2>, arma::mat, StandardCoverTree>
      coverTreeSearch(&referenceTree, false, 0.05);

  arma::Mat<size_t> neighborsCoverTree;
  arma::mat distancesCoverTree;
  coverTreeSearch.Search(dataset, 15, neighborsCoverTree, distancesCoverTree);

  for (size_t i = 0; i < neighborsCoverTree.n_elem; ++i)
    REQUIRE_RELATIVE_ERR(distancesCoverTree[i], distancesExact[i], 0.05);
}

/**
 * Test the ball tree single-tree furthest-neighbors method against the exact
 * method.  This uses only a random reference dataset.
 *
 * Errors are produced if the results are not according to relative error.
 */
BOOST_AUTO_TEST_CASE(SingleBallTreeTest)
{
  arma::mat dataset;
  dataset.randu(75, 1000); // 75 dimensional, 1000 points.

  KFN exact(dataset);
  arma::Mat<size_t> neighborsExact;
  arma::mat distancesExact;
  exact.Search(dataset, 15, neighborsExact, distancesExact);

  NeighborSearch<FurthestNeighborSort, EuclideanDistance, arma::mat, BallTree>
      ballTreeSearch(dataset, false, true, 0.05);

  arma::Mat<size_t> neighborsBallTree;
  arma::mat distancesBallTree;
  ballTreeSearch.Search(dataset, 15, neighborsBallTree, distancesBallTree);

  for (size_t i = 0; i < neighborsBallTree.n_elem; ++i)
    REQUIRE_RELATIVE_ERR(distancesBallTree(i), distancesExact(i), 0.05);
}

/**
 * Test the ball tree dual-tree furthest neighbors method against the exact
 * method.
 *
 * Errors are produced if the results are not according to relative error.
 */
BOOST_AUTO_TEST_CASE(DualBallTreeTest)
{
  arma::mat dataset;
  data::Load("test_data_3_1000.csv", dataset);

  KFN exact(dataset);
  arma::Mat<size_t> neighborsExact;
  arma::mat distancesExact;
  exact.Search(15, neighborsExact, distancesExact);

  NeighborSearch<FurthestNeighborSort, EuclideanDistance, arma::mat, BallTree>
      ballTreeSearch(dataset, false, false, 0.05);
  arma::Mat<size_t> neighborsBallTree;
  arma::mat distancesBallTree;
  ballTreeSearch.Search(15, neighborsBallTree, distancesBallTree);

  for (size_t i = 0; i < neighborsBallTree.n_elem; ++i)
    REQUIRE_RELATIVE_ERR(distancesBallTree(i), distancesExact(i), 0.05);
}

BOOST_AUTO_TEST_SUITE_END();
