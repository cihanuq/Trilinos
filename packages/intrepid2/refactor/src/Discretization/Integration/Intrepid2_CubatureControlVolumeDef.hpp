// @HEADER
// ************************************************************************
//
//                           Intrepid2 Package
//                 Copyright (2007) Sandia Corporation
//
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact Kyungjoo Kim  (kyukim@sandia.gov), or
//                    Mauro Perego  (mperego@sandia.gov)
//
// ************************************************************************
// @HEADER

/** \file   Intrepid_CubatureControlVolumeDef.hpp
    \brief  Header file for the Intrepid2::CubatureControlVolume class.
    \author Created by K. Peterson, P. Bochev and D. Ridzal.
            Kokkorized by Kyungjoo Kim
*/

#ifndef __INTREPID2_CUBATURE_CONTROLVOLUME_DEF_HPP__
#define __INTREPID2_CUBATURE_CONTROLVOLUME_DEF_HPP__

namespace Intrepid2 {

  template <typename SpT, typename PT, typename WT>
  CubatureControlVolume<SpT,PT,WT>::
  CubatureControlVolume(const shards::CellTopology cellTopology) {

    // define primary cell topology with given one
    primaryCellTopo_ = cellTopology;

    // subcell is defined either hex or quad according to dimension
    switch (primaryCellTopo_.getDimension()) {
    case 2:
      subcvCellTopo_ = shards::CellTopology(shards::getCellTopologyData<shards::Quadrilateral<4> >());
      break;
    case 3:
      subcvCellTopo_ = shards::CellTopology(shards::getCellTopologyData<shards::Hexahedron<8> >());
      break;
    }

    // computation order is always one;
    degree_ = 1;

    // create subcell cubature points and weights and cache them
    const ordinal_type subcvDegree = 2;
    auto subcvCubature = DefaultCubatureFactory::create<SpT,PT,WT>(subcvCellTopo_, subcvDegree);

    const auto numSubcvPoints = subcvCubature->getNumPoints();
    const auto subcvDim       = subcvCubature->getDimension();

    subcvCubaturePoints_  = Kokkos::DynRankView<PT,SpT>("CubatureControlVolume::subcvCubaturePoints_",
                                                        numSubcvPoints, subcvDim);
    subcvCubatureWeights_ = Kokkos::DynRankView<WT,SpT>("CubatureControlVolume::subcvCubatureWeights_",
                                                        numSubcvPoints);
    
    subcvCubature->getCubature(subcvCubaturePoints_,
                               subcvCubatureWeights_);
  }
  
  template <typename SpT, typename PT, typename WT>
  void
  CubatureControlVolume<SpT,PT,WT>::
  getCubature( pointViewType  cubPoints,
               weightViewType cubWeights,
               pointViewType  cellCoords ) const {
#ifdef HAVE_INTREPID2_DEBUG
    INTREPID2_TEST_FOR_EXCEPTION( cubPoints.rank() != 3, std::invalid_argument,
                                  ">>> ERROR (CubatureControlVolume): cubPoints must have rank 3 (C,P,D).");
    INTREPID2_TEST_FOR_EXCEPTION( cubWeights.rank() != 2, std::invalid_argument,
                                  ">>> ERROR (CubatureControlVolume): cubWeights must have rank 2 (C,P).");
    INTREPID2_TEST_FOR_EXCEPTION( cellCoords.rank() != 3, std::invalid_argument,
                                  ">>> ERROR (CubatureControlVolume): cellCoords must have rank 3 of (C,P,D).");

    INTREPID2_TEST_FOR_EXCEPTION( cubPoints.dimension(0) != cellCoords.dimension(0) ||
                                  cubPoints.dimension(0) != cubWeights.dimension(0), std::invalid_argument,
                                  ">>> ERROR (CubatureControlVolume): cubPoints, cubWeights and cellCoords dimension(0) are not consistent, numCells");

    INTREPID2_TEST_FOR_EXCEPTION( cubPoints.dimension(1) != cellCoords.dimension(1) ||
                                  cubPoints.dimension(1) != cubWeights.dimension(1), std::invalid_argument,
                                  ">>> ERROR (CubatureControlVolume): cubPoints, cubWeights and cellCoords dimension(1) are not consistent, numNodesPerCell");

    INTREPID2_TEST_FOR_EXCEPTION( cubPoints.dimension(2) != cellCoords.dimension(2) ||
                                  cubPoints.dimension(2) != getDimension(), std::invalid_argument,
                                  ">>> ERROR (CubatureControlVolume): cubPoints, cellCoords, this->getDimension() are not consistent, spaceDim.");
#endif
    typedef Kokkos::DynRankView<PT,SpT> tempPointViewType;

    // get array dimensions
    const auto numCells = cellCoords.dimension(0);
    const auto numNodesPerCell = cellCoords.dimension(1);
    const auto spaceDim = cellCoords.dimension(2);

    const auto numNodesPerSubcv = subcvCellTopo_.getNodeCount();
    tempPointViewType subcvCoords("CubatureControlVolume::subcvCoords_",
                                  numCells, numNodesPerCell, numNodesPerSubcv, spaceDim);
    CellTools<SpT>::getSubcvCoords(subcvCoords,
                                   cellCoords,
                                   primaryCellTopo_);

    const auto numSubcvPoints = subcvCubaturePoints_.dimension(0);    
    tempPointViewType subcvJacobian("CubatureControlVolume::subcvJacobian_",
                                    numCells, numNodesPerCell, numSubcvPoints, spaceDim, spaceDim);
    
    tempPointViewType subcvJacobianDet("CubatureControlVolume::subcvJacobDet_",
                                       numCells, numNodesPerCell, numSubcvPoints);
    
    // numNodesPerCell is maximum 8; this repeated run is necessary because of cell tools input consideration
    for (auto node=0;node<numNodesPerCell;++node) {
      auto subcvJacobianNode    = Kokkos::subdynrankview(subcvJacobian,    Kokkos::ALL(), node, Kokkos::ALL(), Kokkos::ALL(), Kokkos::ALL());
      auto subcvCoordsNode      = Kokkos::subdynrankview(subcvCoords,      Kokkos::ALL(), node, Kokkos::ALL(), Kokkos::ALL());
      auto subcvJacobianDetNode = Kokkos::subdynrankview(subcvJacobianDet, Kokkos::ALL(), node, Kokkos::ALL());
      
      CellTools<SpT>::setJacobian(subcvJacobianNode,    // C, P, D, D
                                  subcvCubaturePoints_, //    P, D
                                  subcvCoordsNode,      // C, N, D
                                  subcvCellTopo_);

      CellTools<SpT>::setJacobianDet(subcvJacobianDetNode,  // C, P
                                     subcvJacobianNode);    // C, P, D, D
    }
    
    typedef typename ExecSpace<typename pointViewType::execution_space,SpT>::ExecSpaceType ExecSpaceType;
    
    const auto loopSize = numCells;
    Kokkos::RangePolicy<ExecSpaceType,Kokkos::Schedule<Kokkos::Static> > policy(0, loopSize);
    
    typedef Functor<pointViewType,weightViewType,tempPointViewType,tempPointViewType,tempPointViewType> FunctorType;
    Kokkos::parallel_for( policy, FunctorType(cubPoints, 
                                              cubWeights, 
                                              subcvCoords, 
                                              subcvCubatureWeights_,
                                              subcvJacobianDet) );
  } 
    
} 

#endif

