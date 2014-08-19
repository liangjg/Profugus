//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   spn_tpetra/test/tstEnergy_Restriction.cc
 * \author Steven Hamilton
 * \date   Mon Apr 01 12:49:01 2013
 * \brief  Energy Grid Transfer test.
 * \note   Copyright (C) 2014 Oak Ridge National Laboratory, UT-Battelle, LLC.
 */
//---------------------------------------------------------------------------//

#include "gtest/utils_gtest.hh"
#include <SPn/config.h>

#include <vector>

#include "Teuchos_DefaultComm.hpp"
#include "Tpetra_MultiVector.hpp"
#include "Tpetra_Vector.hpp"
#include "Tpetra_Map.hpp"

#include "../Energy_Restriction.hh"

typedef KokkosClassic::SerialNode           Node;
typedef Tpetra::Operator<double,int,int,Node> Tpetra_Op;
typedef Tpetra::MultiVector<double,int,int,Node> Tpetra_MV;
typedef Tpetra::Vector<double,int,int,Node> Tpetra_Vector;
typedef Tpetra::Map<int,int,Node> Tpetra_Map;


//---------------------------------------------------------------------------//
// Test fixture
//---------------------------------------------------------------------------//

TEST(RestrictTest, Even)
{
    int Nv = 50;
    int Ng = 8;

    // Get Teuchos communicator
    Teuchos::RCP<const Teuchos::Comm<int> > comm =
        Teuchos::DefaultComm<int>::getComm();

    // Create Epetra maps
    Teuchos::RCP<Tpetra_Map> map0(new Tpetra_Map(
                Teuchos::OrdinalTraits<int>::invalid(), Nv*Ng  ,0,comm) );
    Teuchos::RCP<Tpetra_Map> map1(new Tpetra_Map(
                Teuchos::OrdinalTraits<int>::invalid(), Nv*Ng/2,0,comm) );

    // Create Epetra vectors
    Teuchos::RCP<Tpetra_MV> vec0( new Tpetra_MV(map0,1) );
    Teuchos::RCP<Tpetra_MV> vec1( new Tpetra_MV(map1,1) );

    std::vector<int> steer(4,2);
    profugus::tpetra::Energy_Restriction restrict0( vec0, vec1, steer );

    double tol=1.e-12;

    // Test restriction
    Teuchos::ArrayRCP<double> fine_data   = vec0->getDataNonConst(0);
    Teuchos::ArrayRCP<double> coarse_data = vec1->getDataNonConst(0);
    for( int i=0; i<vec0->getLocalLength(); ++i )
    {
        fine_data[i] = static_cast<double>(2*i);
    }
    restrict0.apply(*vec0,*vec1);
    for( int i=0; i<vec1->getLocalLength(); ++i )
    {
        EXPECT_SOFTEQ(static_cast<double>(4*i+1),coarse_data[i],tol);
    }
}

//---------------------------------------------------------------------------//
//                        end of tstEnergy_Restriction.cc
//---------------------------------------------------------------------------//
