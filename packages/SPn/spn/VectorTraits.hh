//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   SPn/spn/VectorTraits.hh
 * \author Steven Hamilton
 * \date   Mon Feb 17 13:10:27 2014
 * \brief  VectorTraits class definition.
 * \note   Copyright (C) 2014 Oak Ridge National Laboratory, UT-Battelle, LLC.
 */
//---------------------------------------------------------------------------//

#ifndef SPn_spn_VectorTraits_hh
#define SPn_spn_VectorTraits_hh

#include "Teuchos_RCP.hpp"
#include "Teuchos_ArrayRCP.hpp"
#include "Teuchos_ArrayView.hpp"
#include "Teuchos_OrdinalTraits.hpp"

#include "harness/DBC.hh"
#include "solvers/LinAlgTypedefs.hh"


namespace profugus
{

//===========================================================================//
/*!
 * \class VectorTraits
 * \brief Traits class for Epetra/Tpetra matrices
 */
/*!
 * \example spn/test/tstVectorTraits.cc
 *
 * Test of VectorTraits.
 */
//===========================================================================//

template <class T>
class UndefinedVectorTraits
{
    void NotDefined(){T::this_class_is_missing_a_specialization();}
};

template <class T>
class VectorTraits
{
  public:
    //@{
    //! Typedefs.
    typedef typename T::MAP    Map_t;
    typedef typename T::MV     MV;
    //@}

    static Teuchos::RCP<MV> build_vector(Teuchos::RCP<const Map_t> map,
                                         int num_vecs=1)
    {
        UndefinedVectorTraits<T>::NotDefined();
        return Teuchos::null;
    }

    static int local_length(Teuchos::RCP<const MV> vector)
    {
        UndefinedVectorTraits<T>::NotDefined();
        return 0;
    }

    static int local_size(Teuchos::RCP<const Map_t> map)
    {
        UndefinedVectorTraits<T>::NotDefined();
        return 0;
    }

    static void put_scalar(Teuchos::RCP<MV> vector, double val)
    {
        UndefinedVectorTraits<T>::NotDefined();
    }

    static Teuchos::ArrayRCP<double> get_data_nonconst(
        Teuchos::RCP<MV> vector, int ivec=0)
    {
        UndefinedVectorTraits<T>::NotDefined();
        return Teuchos::ArrayRCP<double>();
    }

    static Teuchos::ArrayRCP<const double> get_data(
        Teuchos::RCP<const MV> vector, int ivec=0)
    {
        UndefinedVectorTraits<T>::NotDefined();
        return Teuchos::ArrayRCP<double>();
    }

    static double dot_product(Teuchos::RCP<const MV> x,
                              Teuchos::RCP<const MV> y)
    {
        UndefinedVectorTraits<T>::NotDefined();
        return Teuchos::ScalarTraits<double>::nan();
    }


};

// Specialization on EpetraTypes
template <>
class VectorTraits<EpetraTypes>
{
  public:
    //@{
    //! Typedefs.
    typedef typename EpetraTypes::MAP    Map_t;
    typedef typename EpetraTypes::MV     MV;
    //@}

    static Teuchos::RCP<MV> build_vector(Teuchos::RCP<const Map_t> map,
                                         int num_vecs=1)
    {

        Teuchos::RCP<MV> x(new MV(*map,num_vecs));
        CHECK( x != Teuchos::null );
        return x;
    }

    static int local_length(Teuchos::RCP<const MV> vector)
    {
        return vector->MyLength();
    }

    static int local_size(Teuchos::RCP<const Map_t> map)
    {
        return map->NumMyElements();
    }

    static void put_scalar(Teuchos::RCP<MV> vector, double val)
    {
        vector->PutScalar(val);
    }

    static Teuchos::ArrayRCP<double> get_data_nonconst(
        Teuchos::RCP<MV> vector, int ivec=0)
    {
        bool has_ownership = false;
        return Teuchos::ArrayRCP<double>((*vector)[ivec],0,
            vector->MyLength(), has_ownership);
    }

    static Teuchos::ArrayRCP<const double> get_data(
        Teuchos::RCP<const MV> vector, int ivec=0)
    {
        bool has_ownership = false;
        return Teuchos::ArrayRCP<const double>(
            (*vector)[ivec],0,vector->MyLength(),has_ownership);
    }

    static double dot_product(Teuchos::RCP<const MV> x,
                              Teuchos::RCP<const MV> y)
    {
        double dot;
        int err;
        err = x->Dot(*y,&dot);
        CHECK( err == 0 );
        return dot;
    }
};

// Specialization on TpetraTypes
template <>
class VectorTraits<TpetraTypes>
{
  public:
    //@{
    //! Typedefs.
    typedef typename TpetraTypes::MAP    Map_t;
    typedef typename TpetraTypes::MV     MV;
    //@}

    static Teuchos::RCP<MV> build_vector(Teuchos::RCP<const Map_t> map,
                                         int num_vecs=1)
    {

        Teuchos::RCP<MV> x(new MV(map,num_vecs));
        CHECK( x != Teuchos::null );
        return x;
    }

    static int local_length(Teuchos::RCP<const MV> vector)
    {
        return vector->getLocalLength();
    }

    static int local_size(Teuchos::RCP<const Map_t> map)
    {
        return map->getNodeNumElements();
    }

    static void put_scalar(Teuchos::RCP<MV> vector, double val)
    {
        vector->putScalar(val);
    }

    static Teuchos::ArrayRCP<double> get_data_nonconst(
        Teuchos::RCP<MV> vector, int ivec=0)
    {
        return vector->getDataNonConst(ivec);
    }

    static Teuchos::ArrayRCP<const double> get_data(
        Teuchos::RCP<const MV> vector, int ivec=0)
    {
        return vector->getData(ivec);
    }

    static double dot_product(Teuchos::RCP<const MV> x,
                              Teuchos::RCP<const MV> y)
    {
        Teuchos::ArrayRCP<double> dots(1,0.0);
        x->dot(*y,dots());
        return dots[0];
    }
};

} // end namespace profugus

#endif // SPn_spn_VectorTraits_hh

//---------------------------------------------------------------------------//
//                 end of VectorTraits.hh
//---------------------------------------------------------------------------//
