/**
 *  The MIT License (MIT)
 *
 *  Copyright (c) 2016 Kyle Hollins Wray, University of Massachusetts
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of
 *  this software and associated documentation files (the "Software"), to deal in
 *  the Software without restriction, including without limitation the rights to
 *  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 *  the Software, and to permit persons to whom the Software is furnished to do so,
 *  subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 *  FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 *  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 *  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 *  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include "algorithms/mdp_vi_cpu.h"

#include <gtest/gtest.h>

#include "utilities/mdp_model_cpu.h"
#include "error_codes.h"
#include "constants.h"


nova::MDP *create_simple_mdp()
{
    nova::MDP *mdp = new nova::MDP();

    mdp->n = 1;
    mdp->ns = 1;
    mdp->m = 1;

    mdp->S = new int[1];
    mdp->S[0] = 0;

    mdp->T = new float[1];
    mdp->T[0] = 1.0f;

    mdp->R = new float[1];
    mdp->R[0] = 1.0f;

    mdp->gamma = 0.9f;
    mdp->horizon = 3;
    mdp->epsilon = 0.1f;

    return mdp;
}


TEST(MDPVICPU, initialization)
{
    nova::MDP mdp;
    mdp.n = 2;

    nova::MDPVICPU vi;
    vi.VInitial = nullptr;

    int result = nova::mdp_vi_initialize_cpu(&mdp, &vi);
    EXPECT_EQ(result, NOVA_SUCCESS);

    EXPECT_EQ(vi.VInitial, nullptr);
    EXPECT_EQ(vi.currentHorizon, 0);

    EXPECT_NE(vi.V, nullptr);
    EXPECT_EQ(vi.V[0], 0.0f);
    EXPECT_EQ(vi.V[1], 0.0f);

    EXPECT_NE(vi.VPrime, nullptr);
    EXPECT_EQ(vi.VPrime[0], 0.0f);
    EXPECT_EQ(vi.VPrime[1], 0.0f);

    EXPECT_NE(vi.pi, nullptr);
    EXPECT_EQ(vi.pi[0], 0);
    EXPECT_EQ(vi.pi[1], 0);

    delete [] vi.V;
    delete [] vi.VPrime;
    delete [] vi.pi;

    vi.VInitial = new float[2];
    vi.VInitial[0] = -1.0f;
    vi.VInitial[1] = 1.0f;

    result = nova::mdp_vi_initialize_cpu(&mdp, &vi);
    EXPECT_EQ(result, NOVA_SUCCESS);

    EXPECT_EQ(vi.currentHorizon, 0);

    EXPECT_NE(vi.V, nullptr);
    EXPECT_EQ(vi.V[0], -1.0f);
    EXPECT_EQ(vi.V[1], 1.0f);

    EXPECT_NE(vi.VPrime, nullptr);
    EXPECT_EQ(vi.VPrime[0], -1.0f);
    EXPECT_EQ(vi.VPrime[1], 1.0f);

    EXPECT_NE(vi.pi, nullptr);
    EXPECT_EQ(vi.pi[0], 0);
    EXPECT_EQ(vi.pi[1], 0);

    delete [] vi.V;
    delete [] vi.VPrime;
    delete [] vi.pi;

    delete [] vi.VInitial;
}


TEST(MDPVICPU, badInitializations)
{
    nova::MDP mdp;
    mdp.n = 0;

    nova::MDPVICPU vi;
    vi.VInitial = new float[2];
    vi.VInitial[0] = -1.0f;
    vi.VInitial[1] = 1.0f;

    int result = 0;

    bool requiresCleanup = false;

    result = nova::mdp_vi_initialize_cpu(nullptr, &vi);
    EXPECT_EQ(result, NOVA_ERROR_INVALID_DATA);
    if (result != NOVA_ERROR_INVALID_DATA) {
        requiresCleanup = true;
    }

    result = nova::mdp_vi_initialize_cpu(&mdp, nullptr);
    EXPECT_EQ(result, NOVA_ERROR_INVALID_DATA);
    if (result != NOVA_ERROR_INVALID_DATA) {
        requiresCleanup = true;
    }

    result = nova::mdp_vi_initialize_cpu(&mdp, &vi);
    EXPECT_EQ(result, NOVA_ERROR_INVALID_DATA);
    if (result != NOVA_ERROR_INVALID_DATA) {
        requiresCleanup = true;
    }

    if (requiresCleanup) {
        result = nova::mdp_vi_uninitialize_cpu(&mdp, &vi);
        EXPECT_EQ(result, NOVA_SUCCESS);
    }

    delete [] vi.VInitial;
}


TEST(MDPVICPU, execution)
{
    nova::MDP *mdp = create_simple_mdp();
    nova::MDPVICPU vi;
    nova::MDPValueFunction *policy = nullptr;

    int result = nova::mdp_vi_execute_cpu(mdp, &vi, policy);
    EXPECT_EQ(result, NOVA_SUCCESS);

    EXPECT_NE(policy, nullptr);

    if (policy != nullptr) {
        EXPECT_EQ(policy->n, 1);
        EXPECT_EQ(policy->m, 1);

        EXPECT_EQ(policy->r, 0);
        EXPECT_EQ(policy->S, nullptr);

        EXPECT_NE(policy->V, nullptr);
        if (policy->V != nullptr) {
            EXPECT_EQ(policy->V[0], 2.71f);
        }

        EXPECT_NE(policy->pi, nullptr);
        if (policy->pi != nullptr) {
            EXPECT_EQ(policy->pi[0], 0);
        }
    }

    if (policy != nullptr) {
        result = nova::mdp_value_function_uninitialize(policy);
        EXPECT_EQ(result, NOVA_SUCCESS);
    }

    result = nova::mdp_uninitialize_cpu(mdp);
    EXPECT_EQ(result, NOVA_SUCCESS);

    delete mdp;
}


TEST(MDPVICPU, badExecution)
{
    nova::MDP *mdp = create_simple_mdp();
    nova::MDPVICPU vi;
    nova::MDPValueFunction *policy = nullptr;

    int result = nova::mdp_vi_execute_cpu(nullptr, &vi, policy);
    EXPECT_EQ(result, NOVA_ERROR_INVALID_DATA);

    result = nova::mdp_vi_execute_cpu(mdp, nullptr, policy);
    EXPECT_EQ(result, NOVA_ERROR_INVALID_DATA);

    policy = new nova::MDPValueFunction();
    result = nova::mdp_vi_execute_cpu(mdp, &vi, policy);
    EXPECT_EQ(result, NOVA_ERROR_INVALID_DATA);
    delete policy;

    mdp->n = 0;
    result = nova::mdp_vi_execute_cpu(mdp, &vi, policy);
    EXPECT_EQ(result, NOVA_ERROR_INVALID_DATA);
    mdp->n = 1;

    mdp->ns = 0;
    result = nova::mdp_vi_execute_cpu(mdp, &vi, policy);
    EXPECT_EQ(result, NOVA_ERROR_INVALID_DATA);
    mdp->ns = 1;

    mdp->m = 0;
    result = nova::mdp_vi_execute_cpu(mdp, &vi, policy);
    EXPECT_EQ(result, NOVA_ERROR_INVALID_DATA);
    mdp->m = 1;

    void *tempAddress = nullptr;

    tempAddress = mdp->S;
    mdp->S = nullptr;
    result = nova::mdp_vi_execute_cpu(mdp, &vi, policy);
    EXPECT_EQ(result, NOVA_ERROR_INVALID_DATA);
    mdp->S = (int *)tempAddress;

    tempAddress = mdp->T;
    mdp->T = nullptr;
    result = nova::mdp_vi_execute_cpu(mdp, &vi, policy);
    EXPECT_EQ(result, NOVA_ERROR_INVALID_DATA);
    mdp->T = (float *)tempAddress;

    tempAddress = mdp->R;
    mdp->R = nullptr;
    result = nova::mdp_vi_execute_cpu(mdp, &vi, policy);
    EXPECT_EQ(result, NOVA_ERROR_INVALID_DATA);
    mdp->R = (float *)tempAddress;

    mdp->gamma = -1.0f;
    result = nova::mdp_vi_execute_cpu(mdp, &vi, policy);
    EXPECT_EQ(result, NOVA_ERROR_INVALID_DATA);
    mdp->gamma = 0.9f;

    mdp->gamma = 2.0f;
    result = nova::mdp_vi_execute_cpu(mdp, &vi, policy);
    EXPECT_EQ(result, NOVA_ERROR_INVALID_DATA);
    mdp->gamma = 0.9f;

    mdp->horizon = 0;
    result = nova::mdp_vi_execute_cpu(mdp, &vi, policy);
    EXPECT_EQ(result, NOVA_ERROR_INVALID_DATA);
    mdp->horizon = 3;

    if (policy != nullptr) {
        result = nova::mdp_value_function_uninitialize(policy);
        EXPECT_EQ(result, NOVA_SUCCESS);
    }

    delete mdp;
}


TEST(MDPVICPU, uninitialization)
{
    nova::MDP mdp;

    nova::MDPVICPU vi;
    vi.V = new float[1];
    vi.VPrime = new float[1];
    vi.pi = new unsigned int[1];

    int result = nova::mdp_vi_uninitialize_cpu(&mdp, &vi);
    EXPECT_EQ(result, NOVA_SUCCESS);

    EXPECT_EQ(vi.currentHorizon, 0);
    EXPECT_EQ(vi.V, nullptr);
    EXPECT_EQ(vi.VPrime, nullptr);
    EXPECT_EQ(vi.pi, nullptr);

    if (vi.V != nullptr) {
        delete [] vi.V;
    }
    if (vi.VPrime != nullptr) {
        delete [] vi.VPrime;
    }
    if (vi.pi != nullptr) {
        delete [] vi.pi;
    }
}


TEST(MDPVICPU, badUninitialization)
{
    nova::MDP mdp;

    int result = nova::mdp_vi_uninitialize_cpu(nullptr, nullptr);
    EXPECT_EQ(result, NOVA_ERROR_INVALID_DATA);

    result = nova::mdp_vi_uninitialize_cpu(&mdp, nullptr);
    EXPECT_EQ(result, NOVA_ERROR_INVALID_DATA);
}


// For performance reasons, there is no checking here, so we simply
// are checking valid mathematics.
TEST(MDPVICPU, update)
{
    nova::MDP *mdp = create_simple_mdp();
    nova::MDPVICPU vi;

    int result = nova::mdp_vi_initialize_cpu(mdp, &vi);
    EXPECT_EQ(result, NOVA_SUCCESS);

    vi.currentHorizon = 0;
    vi.pi[0] = 1337;

    result = nova::mdp_vi_update_cpu(mdp, &vi);
    EXPECT_EQ(result, NOVA_SUCCESS);
    EXPECT_EQ(vi.currentHorizon, 1);
    EXPECT_EQ(vi.V[0], 0.0f);
    EXPECT_EQ(vi.VPrime[0], 1.0f);
    EXPECT_EQ(vi.pi[0], 0);

    result = nova::mdp_vi_update_cpu(mdp, &vi);
    EXPECT_EQ(result, NOVA_SUCCESS);
    EXPECT_EQ(vi.currentHorizon, 2);
    EXPECT_EQ(vi.V[0], 1.9f);
    EXPECT_EQ(vi.VPrime[0], 1.0f);
    EXPECT_EQ(vi.pi[0], 0);

    result = nova::mdp_vi_update_cpu(mdp, &vi);
    EXPECT_EQ(result, NOVA_SUCCESS);
    EXPECT_EQ(vi.currentHorizon, 3);
    EXPECT_EQ(vi.V[0], 1.9f);
    EXPECT_EQ(vi.VPrime[0], 2.71f);
    EXPECT_EQ(vi.pi[0], 0);

    result = nova::mdp_vi_uninitialize_cpu(mdp, &vi);
    EXPECT_EQ(result, NOVA_SUCCESS);

    result = nova::mdp_uninitialize_cpu(mdp);
    EXPECT_EQ(result, NOVA_SUCCESS);

    delete mdp;
}


// There is no bad update because for performance reasons, we do not
// check or sanitize input to this function.
//TEST(MDPVICPU, badUpdate) { }


TEST(MDPVICPU, getPolicy)
{
    nova::MDP mdp;
    mdp.n = 2;
    mdp.m = 3;

    nova::MDPVICPU vi;
    vi.V = new float[2];
    vi.V[0] = 10;
    vi.V[1] = 20;

    vi.VPrime = new float[2];
    vi.VPrime[0] = 30;
    vi.VPrime[1] = 40;

    vi.pi = new unsigned int[2];
    vi.pi[0] = 50;
    vi.pi[1] = 60;

    for (unsigned int i = 0; i < 5; i++) {
        vi.currentHorizon = i;

        nova::MDPValueFunction *policy = nullptr;

        int result = nova::mdp_vi_get_policy_cpu(&mdp, &vi, policy);
        EXPECT_EQ(result, NOVA_SUCCESS);

        EXPECT_NE(policy, nullptr);

        if (policy != nullptr) {
            EXPECT_EQ(policy->n, mdp.n);
            EXPECT_EQ(policy->m, mdp.m);

            EXPECT_EQ(policy->r, 0);
            EXPECT_EQ(policy->S, nullptr);

            EXPECT_NE(policy->V, nullptr);
            if (i % 2 == 0) {
                EXPECT_EQ(policy->V[0], 10);
                EXPECT_EQ(policy->V[1], 20);
            } else {
                EXPECT_EQ(policy->V[0], 30);
                EXPECT_EQ(policy->V[1], 40);
            }

            EXPECT_NE(policy->V, nullptr);
            EXPECT_EQ(policy->pi[0], 50);
            EXPECT_EQ(policy->pi[1], 60);

            result = nova::mdp_value_function_uninitialize(policy);
            EXPECT_EQ(result, NOVA_SUCCESS);
        }
    }

    delete [] vi.V;
    delete [] vi.VPrime;
    delete [] vi.pi;
}


TEST(MDPVICPU, badGetPolicy)
{
    nova::MDP mdp;
    nova::MDPVICPU vi;
    nova::MDPValueFunction *policy = new nova::MDPValueFunction();

    int result = nova::mdp_vi_get_policy_cpu(nullptr, nullptr, policy);
    EXPECT_EQ(result, NOVA_ERROR_INVALID_DATA);

    result = nova::mdp_vi_get_policy_cpu(&mdp, nullptr, policy);
    EXPECT_EQ(result, NOVA_ERROR_INVALID_DATA);

    result = nova::mdp_vi_get_policy_cpu(&mdp, &vi, policy);
    EXPECT_EQ(result, NOVA_ERROR_INVALID_DATA);

    delete policy;
}


int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

