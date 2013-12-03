//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   comm/test/tstPingPong.cc
 * \author Thomas M. Evans
 * \date   Wed Jan  2 16:36:26 2008
 * \brief  Ping Pong communication test.
 * \note   Copyright (C) 2008 Oak Ridge National Laboratory, UT-Battelle, LLC.
 */
//---------------------------------------------------------------------------//

#include <iostream>
#include <vector>
#include <cmath>
#include <sstream>
#include <cstdlib>

#include "harness/DBC.hh"
#include "release/Release.hh"
#include "harness/Soft_Equivalence.hh"
#include "../global.hh"
#include "../Parallel_Unit_Test.hh"

using namespace std;
using nemesis::Parallel_Unit_Test;
using nemesis::soft_equiv;

int node  = 0;
int nodes = 0;

using nemesis::Request;
using nemesis::send;
using nemesis::receive;
using nemesis::send_async;
using nemesis::receive_async;
using nemesis::probe;
using nemesis::blocking_probe;

#define ITFAILS ut.failure(__LINE__);
#define UNIT_TEST(a) if (!(a)) ut.failure(__LINE__);

//---------------------------------------------------------------------------//
// TESTS
//---------------------------------------------------------------------------//

void blocking_ping_pong(Parallel_Unit_Test &ut)
{
    if (nemesis::nodes() != 2) return;

    char   c = 0;
    int    i = 0;
    long   l = 0;
    float  f = 0;
    double d = 0;

    // assign on node 0
    if (nemesis::node() == 0)
    {
        c = 'A';
        i = 1;
        l = 1000;
        f = 1.5;
        d = 2.5;

        // send out data
        // Test both active and depricated forms of the send command.
        send(&c, 1, 1);
        send(&i, 1, 1);  // This form is deprecated.
        send(&l, 1, 1);
        send(&f, 1, 1);
        send(&d, 1, 1);

        // receive back
        // Test both active and depricated forms of the receive command.
        receive(&c, 1, 1);
        receive(&i, 1, 1); // This form is deprecated.
        receive(&l, 1, 1);
        receive(&f, 1, 1);
        receive(&d, 1, 1);

        // check values
        if (c != 'B')             ITFAILS;
        if (i != 2)               ITFAILS;
        if (l != 2000)            ITFAILS;
        if (!soft_equiv(f, 2.5f)) ITFAILS;
        if (!soft_equiv(d, 3.5))  ITFAILS;
    }

    // receive and send on node 1
    if (nemesis::node() == 1)
    {
        // receive from node 0
        receive(&c, 1, 0);
        receive(&i, 1, 0);
        receive(&l, 1, 0);
        receive(&f, 1, 0);
        receive(&d, 1, 0);

        // check values
        if (c != 'A')             ITFAILS;
        if (i != 1)               ITFAILS;
        if (l != 1000)            ITFAILS;
        if (!soft_equiv(f, 1.5f)) ITFAILS;
        if (!soft_equiv(d, 2.5))  ITFAILS;

        // assign new values
        c = 'B';
        i = 2;
        l = 2000;
        f = 2.5;
        d = 3.5;

        // send them back
        send(&c, 1, 0);
        send(&i, 1, 0);
        send(&l, 1, 0);
        send(&f, 1, 0);
        send(&d, 1, 0);
    }

    nemesis::global_barrier();

    if (ut.numFails == 0)
    {
        ostringstream m;
        m << "Blocking Send/Recv communication ok on " << nemesis::node();
        ut.passes(m.str());
    }
}
//---------------------------------------------------------------------------//

void non_blocking_ping_pong(Parallel_Unit_Test &ut)
{
    if (nemesis::nodes() != 2) return;

    char   c = 0;
    int    i = 0;
    long   l = 0;
    float  f = 0;
    double d = 0;

    char   cr = 0;
    int    ir = 0;
    long   lr = 0;
    float  fr = 0;
    double dr = 0;

    // send requests
    Request crs, irs, lrs, frs, drs;

    // receive requests
    Request crr, irr, lrr, frr, drr;

    // assign on node 0
    if (nemesis::node() == 0)
    {
        // post receives
        // Test two forms of the receive_async command plus one deprecated
        // form (namespace C4::)
        receive_async(crr, &cr, 1, 1);
        irr = receive_async(&ir, 1, 1);
        receive_async(lrr, &lr, 1, 1);
        frr = receive_async(&fr, 1, 1);
        receive_async(drr, &dr, 1, 1);

        // give values to the send data
        c = 'A';
        i = 1;
        l = 1000;
        f = 1.5;
        d = 2.5;

        // send out data
        // Test two forms of the send_async command plus one deprecated
        // form (namespace C4::)
        send_async(crs, &c, 1, 1);
        irs = send_async( &i, 1, 1);
        send_async(lrs, &l, 1, 1);
        frs = send_async(&f,1,1);
        send_async(drs, &d, 1, 1);

        // wait for sends to be finished
        crs.wait();
        irs.wait();
        lrs.wait();
        frs.wait();
        drs.wait();

        // wait on receives and check
        crr.wait();
        irr.wait();
        lrr.wait();
        frr.wait();
        drr.wait();

        // check values
        if (cr != 'B')             ITFAILS;
        if (ir != 2)               ITFAILS;
        if (lr != 2000)            ITFAILS;
        if (!soft_equiv(fr, 2.5f)) ITFAILS;
        if (!soft_equiv(dr, 3.5))  ITFAILS;

        if (crr.count() != 1) ITFAILS;
        if (irr.count() != sizeof(int)) ITFAILS;
        if (lrr.count() != sizeof(long)) ITFAILS;
        if (frr.count() != sizeof(float)) ITFAILS;
        if (drr.count() != sizeof(double)) ITFAILS;
    }

    // receive and send on node 1
    if (nemesis::node() == 1)
    {
        // post receives
        // Test both function that provide equivalent functionality.
        receive_async(crr, &cr, 1, 0);
        irr = receive_async( &ir, 1, 0);
        receive_async(lrr, &lr, 1, 0);
        receive_async(frr, &fr, 1, 0);
        receive_async(drr, &dr, 1, 0);

        // check that all are inuse
        if (!crr.inuse()) ITFAILS;
        if (!irr.inuse()) ITFAILS;
        if (!lrr.inuse()) ITFAILS;
        if (!frr.inuse()) ITFAILS;
        if (!drr.inuse()) ITFAILS;

        // check on receives
        int done = 0;
        while (done < 5)
        {
            if (crr.complete()) done++;
            if (irr.complete()) done++;
            if (lrr.complete()) done++;
            if (frr.complete()) done++;
            if (drr.complete()) done++;
        }

        if (cr != 'A')             ITFAILS;
        if (ir != 1)               ITFAILS;
        if (lr != 1000)            ITFAILS;
        if (!soft_equiv(fr, 1.5f)) ITFAILS;
        if (!soft_equiv(dr, 2.5))  ITFAILS;

        // assign new values
        c = 'B';
        i = 2;
        l = 2000;
        f = 2.5;
        d = 3.5;

        // send them back
        // Test both function that provide equivalent functionality.

        send_async(crs, &c, 1, 0);
        irs = send_async( &i, 1, 0);
        send_async(lrs, &l, 1, 0);
        send_async(frs, &f, 1, 0);
        send_async(drs, &d, 1, 0);

        // wait for sends to be finished
        crs.wait();
        irs.wait();
        lrs.wait();
        frs.wait();
        drs.wait();
    }

    nemesis::global_barrier();

    // check that all requests are done
    if (crs.inuse()) ITFAILS;
    if (irs.inuse()) ITFAILS;
    if (lrs.inuse()) ITFAILS;
    if (frs.inuse()) ITFAILS;
    if (drs.inuse()) ITFAILS;

    if (crr.inuse()) ITFAILS;
    if (irr.inuse()) ITFAILS;
    if (lrr.inuse()) ITFAILS;
    if (frr.inuse()) ITFAILS;
    if (drr.inuse()) ITFAILS;

    if (ut.numFails == 0)
    {
        ostringstream m;
        m << "Non-blocking Send/Recv communication ok on " << nemesis::node();
        ut.passes(m.str());
    }
}

//---------------------------------------------------------------------------//
// Test the Request.free() function.
// After a asynchronous receive command is given, it can be terminated by
// using the free() command.
//---------------------------------------------------------------------------//

void tstReq_free(Parallel_Unit_Test &ut)
{
    if (nemesis::nodes() != 2) return;

    char   cr = 0;

    // receive requests
    Request crr;

    // assign on node 0
    if (nemesis::node() == 0)
    {
        // post receives
        receive_async(crr, &cr, 1, 1);

        // void the receive request.
        crr.free();
    }

    return;
}

//---------------------------------------------------------------------------//

void probe_ping_pong(Parallel_Unit_Test &ut)
{
    if (nemesis::nodes() != 2) return;

    int   i = 0;

    int   ir = 0;

    // send requests
    Request irs;

    // receive requests
    Request irr;

    // Only 2 procs so, dest is either 0 or 1.
    int dest( std::abs( nemesis::node() - 1 ) );

    // assign on node 1
    if (nemesis::node() == 1)
    {

        // give values to the send data
        i = 2;

        // send out data
        send_async(irs, &i, 1, dest);
    }

    // receive and send on node 0
    if (nemesis::node() == 0)
    {
        // test the probe function
        int message_size;
        for (;;)
        {
            if (probe(dest, nemesis::Comm_Traits<int*>::tag, message_size))
            {
                if (message_size==sizeof(int))
                {
                    ut.passes("Probe returned correct size");
                }
                else
                {
                    ut.failure("Probe returned WRONG size");
                }
                break;
            }
        }

        // test the blocking probe function
        blocking_probe(dest, nemesis::Comm_Traits<int*>::tag, message_size);
        if (message_size==sizeof(int))
        {
            ut.passes("Blocking probe returned correct size");
        }
        else
        {
            ut.failure("Blocking probe returned WRONG size");
        }

        // post receives
        receive_async(irr, &ir, 1, dest);
    }

    nemesis::global_barrier();

    if (ut.numFails == 0)
    {
        ostringstream m;
        m << "Probe communication ok on " << nemesis::node();
        ut.passes(m.str());
    }
}

//---------------------------------------------------------------------------//

int main(int argc, char *argv[])
{
    Parallel_Unit_Test ut(argc, argv, nemesis::release::short_version);

    node  = nemesis::node();
    nodes = nemesis::nodes();

    try
    {
        // >>> UNIT TESTS
        int gpass = 0;
        int gfail = 0;

        if (nodes == 2)
        {
            blocking_ping_pong(ut);
            gpass += ut.numPasses;
            gfail += ut.numFails;
            ut.reset();

            non_blocking_ping_pong(ut);
            gpass += ut.numPasses;
            gfail += ut.numFails;
            ut.reset();

            probe_ping_pong(ut);
            gpass += ut.numPasses;
            gfail += ut.numFails;
            ut.reset();

            tstReq_free(ut);
            gpass += ut.numPasses;
            gfail += ut.numFails;
            ut.reset();
        }
        else
        {
            gpass += 1;
        }

        // add up global passes and fails
        nemesis::global_sum(gpass);
        nemesis::global_sum(gfail);
        ut.numPasses = gpass;
        ut.numFails  = gfail;
    }
    catch (std::exception &err)
    {
        std::cout << "ERROR: While testing tstPingPong, "
                  << err.what()
                  << endl;
        ut.numFails++;
    }
    catch( ... )
    {
        std::cout << "ERROR: While testing tstPingPong, "
                  << "An unknown exception was thrown."
                  << endl;
        ut.numFails++;
    }
    return ut.numFails;
}

//---------------------------------------------------------------------------//
//                        end of tstPingPong.cc
//---------------------------------------------------------------------------//
