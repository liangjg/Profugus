//---------------------------------*-CUDA-*----------------------------------//
/*!
 * \file   cuda_mc/test/Collision_Tally_Tester.cu
 * \author Stuart Slattery
 * \note   Copyright (C) 2013 Oak Ridge National Laboratory, UT-Battelle, LLC.
 */
//---------------------------------------------------------------------------//

#include "Collision_Tally_Tester.hh"

#include "cuda_utils/Hardware.hh"
#include "cuda_utils/CudaDBC.hh"

#include <Teuchos_Array.hpp>

#include <cuda_runtime.h>

//---------------------------------------------------------------------------//
// CUDA Kernels
//---------------------------------------------------------------------------//
__global__ 
void set_wt_kernel( Collision_Tally_Tester::Particle_Vector* vector, 
		    double* wt )
{
    int i = threadIdx.x + blockIdx.x * blockDim.x;
    vector->set_wt( i, wt[i] );
}

//---------------------------------------------------------------------------//
__global__ 
void set_event_kernel( Collision_Tally_Tester::Particle_Vector* vector, 
		       typename Collision_Tally_Tester::Event_t* event )
{
    int i = threadIdx.x + blockIdx.x * blockDim.x;
    vector->set_event( i, event[i] );
}

//---------------------------------------------------------------------------//
__global__ 
void set_geo_state_kernel( 
    Collision_Tally_Tester::Particle_Vector* vector, 
    typename Collision_Tally_Tester::Geo_State_t* geo_state )
{
    int i = threadIdx.x + blockIdx.x * blockDim.x;
    vector->geo_state( i ) = geo_state[i];
}

//---------------------------------------------------------------------------//
__global__ 
void set_batch_kernel( Collision_Tally_Tester::Particle_Vector* vector, 
		       int* batch )
{
    int i = threadIdx.x + blockIdx.x * blockDim.x;
    vector->set_batch( i, batch[i] );
}

//---------------------------------------------------------------------------//
__global__ 
void live_kernel( Collision_Tally_Tester::Particle_Vector* vector )
{
    int i = threadIdx.x + blockIdx.x * blockDim.x;
    vector->live( i );
}

//---------------------------------------------------------------------------//
// Collision_Tally_Tester
//---------------------------------------------------------------------------//
Collision_Tally_Tester::Collision_Tally_Tester(  const std::vector<double>& x_edges,
						 const std::vector<double>& y_edges,
						 const std::vector<double>& z_edges,
						 const int num_particle, 
						 const profugus::RNG& rng,
						 const int num_batch )
{
    // Acquire hardware for the test.
    cuda::Hardware<cuda::arch::Device>::acquire();

    // Create the geometry.
    d_geometry = cuda::shared_device_ptr<Geometry>( x_edges, y_edges, z_edges );
    int num_cells = d_geometry.get_host_ptr()->num_cells();

    // Create the particle vector.
    d_particles = cuda::shared_device_ptr<Particle_Vector>( num_particle, rng );

    // Initialize the state of the particle vector. The first half of the
    // cells will have a particle with a collision event, the rest will
    // not. There will be one particle per cell per batch.

    // Cuda launch params.
    int num_threads = 256;
    int num_block = num_particle / num_threads;

    // Set the weight. Weight assigned as a function of the cell id and the
    // batch id.
    Teuchos::Array<double> wt( num_particle );
    for ( int b = 0; b < num_batch; ++b )
    {
	for ( int c = 0; c < num_cells; ++c )
	{
	    wt[ b*num_cells + c ] = (b+1)*(c+1);
	}
    }
    double* device_wt;
    cudaMalloc( (void**) &device_wt, num_particle * sizeof(double) );
    cudaMemcpy( device_wt, wt.getRawPtr(), num_particle * sizeof(double),
		cudaMemcpyHostToDevice );
    set_wt_kernel<<<num_block,num_threads>>>( 
	d_particles.get_device_ptr(), device_wt );
    cudaFree( device_wt );

    // Set the event. If the particle is not in the first cells it does not
    // collide.
    Teuchos::Array<Event_t> events( num_particle );
    for ( int b = 0; b < num_batch; ++b )
    {
	for ( int c = 0; c < num_cells; ++c )
	{
	    if ( c < num_cells / 2 )
	    {
		events[ b*num_cells + c ] = cuda_profugus::events::COLLISION;
	    }
	    else
	    {
		events[ b*num_cells + c ] = cuda_profugus::events::ABSORPTION;
	    }
	}
    }
    Event_t* device_event;
    cudaMalloc( (void**) &device_event, num_particle * sizeof(Event_t) );
    cudaMemcpy( device_event, events.getRawPtr(), num_particle * sizeof(Event_t),
		cudaMemcpyHostToDevice );
    set_event_kernel<<<num_block,num_threads>>>( 
	d_particles.get_device_ptr(), device_event );
    cudaFree( device_event );

    // Set the batch
    Teuchos::Array<int> batch( num_particle );
    for ( int b = 0; b < num_batch; ++b )
    {
	for ( int c = 0; c < num_cells; ++c )
	{
	    batch[ b*num_cells + c ] = b;
	}
    }
    int* device_batch;
    cudaMalloc( (void**) &device_batch, num_particle * sizeof(int) );
    cudaMemcpy( device_batch, batch.getRawPtr(), num_particle * sizeof(int),
		cudaMemcpyHostToDevice );
    set_batch_kernel<<<num_block,num_threads>>>( 
	d_particles.get_device_ptr(), device_batch );
    cudaFree( device_batch );

    // Set the geometry state. One particle per cell per batch.
    Teuchos::Array<Geo_State_t> geo_state( num_particle );
    for ( int b = 0; b < num_batch; ++b )
    {
	for ( int c = 0; c < num_cells; ++c )
	{
	    int i, j, k;
	    d_geometry.get_host_ptr()->mesh().cardinal( c, i, j, k );
	    geo_state[ b*num_cells + c ].ijk.i = i;
	    geo_state[ b*num_cells + c ].ijk.j = j;
	    geo_state[ b*num_cells + c ].ijk.k = k;
	}
    }
    Geo_State_t* device_geo_state;
    cudaMalloc( (void**) &device_geo_state, num_particle * sizeof(Geo_State_t) );
    cudaMemcpy( device_geo_state, geo_state.getRawPtr(), num_particle * sizeof(Geo_State_t),
		cudaMemcpyHostToDevice );
    set_geo_state_kernel<<<num_block,num_threads>>>( 
	d_particles.get_device_ptr(), device_geo_state );
    cudaFree( device_geo_state );

    // Set the particles to alive.
    live_kernel<<<num_block,num_threads>>>( d_particles.get_device_ptr() );

    // sort the particles by event.
    d_particles.get_host_ptr()->sort_by_event();
}

//---------------------------------------------------------------------------//
//                 end of cuda_mc/Collision_Tally_Tester.cu
//---------------------------------------------------------------------------//
