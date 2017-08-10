// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "pow.h"

#include "arith_uint256.h"
#include "chain.h"
#include "chainparams.h"
#include "primitives/block.h"
#include "uint256.h"
#include "util.h"
#include <math.h>

unsigned int static DeltaGravityWave(const CBlockIndex* pindexLast, const uint32_t nTime, const Consensus::Params& params) 
{
    /* DeltaGravityWave Difficulty Re-Adjustment Algorithm 
     * By: VIZ core development team
     *
     * Based on a combination of: 
     * -- DarkGravity v3, written by Evan Duffield 
     * -- Delta algorithm, written by Frank (dt_cdog@yahoo.com) with adjustments by Malcolm MacLeod (mmacleod@webmail.co.za)
     * */
    const CBlockIndex *BlockLastSolved = pindexLast;
    const CBlockIndex *BlockReading = pindexLast;
    int64_t nActualTimespan = 0;
    int64_t LastBlockTime = 0;
    int64_t PastBlocksMin = 20; 
    int64_t PastBlocksMax = 4; 
    int64_t CountBlocks = 0;
    arith_uint256 PastDifficultyAverage;
    arith_uint256 PastDifficultyAveragePrev;

    if (BlockLastSolved == NULL || BlockLastSolved->nHeight == 0 || BlockLastSolved->nHeight < PastBlocksMin) 
	{
        return UintToArith256(params.powLimit).GetCompact();
    }

    for (unsigned int i = 1; BlockReading && BlockReading->nHeight > 0; i++) 
	{
        //LogPrintf("pblock hash = %s, i: %d, BlockReading && BlockReading->nHeight == %d\n", 
        //        pblock->GetHash().ToString().c_str(), i, (BlockReading && BlockReading->nHeight));

        if (PastBlocksMax > 0 && i > PastBlocksMax) { break; }
        CountBlocks++;

        if(CountBlocks <= PastBlocksMin) 
		{
            if (CountBlocks == 1) { PastDifficultyAverage.SetCompact(BlockReading->nBits); }
              else { PastDifficultyAverage = ((PastDifficultyAveragePrev * CountBlocks) + (arith_uint256().SetCompact(BlockReading->nBits))) / (CountBlocks + 1); }
            PastDifficultyAveragePrev = PastDifficultyAverage;
        }

        if(LastBlockTime > 0)
		{
            int64_t Diff = (LastBlockTime - BlockReading->GetBlockTime());
            nActualTimespan += Diff;
        }
        LastBlockTime = BlockReading->GetBlockTime();

        if (BlockReading->pprev == NULL) { assert(BlockReading); break; }
        BlockReading = BlockReading->pprev;
    }

    arith_uint256 bnNew(PastDifficultyAverage);

    int64_t _nTargetTimespan = CountBlocks * params.nPowTargetSpacing; 

    if (nActualTimespan < _nTargetTimespan/2)
    {
        nActualTimespan = _nTargetTimespan/2;
    }

    if (nActualTimespan > _nTargetTimespan*2) 
    {
        nActualTimespan = _nTargetTimespan*2;
    }

	arith_uint256 bnOld = bnNew;
   
    // Retarget
    bnNew *= nActualTimespan;
    bnNew /= _nTargetTimespan;

    // DELTA retarget if block time is greater than nLongTimeLimit
    const int64_t nLongTimeLimit = 2 * 16 * 60;
    const int64_t nLongTimeStep = 15 * 60;

    if ((nTime - pindexLast->GetBlockTime()) > nLongTimeLimit)
    {
        // Reduce in a linear fashion at first, and then start to ramp up with a gradual exponential curve (to catch massive hash extinction events)
        int64_t nNumMissedSteps = ((nTime - pindexLast->GetBlockTime()) / nLongTimeStep);
        LogPrintf("nLongTimeLimit check passed, retargeting %u steps due to %f-minute block\n", nNumMissedSteps, (nTime - pindexLast->GetBlockTime())/60);
        if (nNumMissedSteps <= 5)
            bnNew *= nNumMissedSteps;
        else
            bnNew *= 5 + (int64_t)std::floor(std::pow((float)1.14, (float)nNumMissedSteps - 5) + 0.5);
    }

	LogPrintf("DGW: Height %f, NewDiff %08x     nActualTimespan %f    nTargetTimespan %f   Before %s, After %s \r\n",		
		(double)pindexLast->nHeight, bnNew.GetCompact(),	(double)nActualTimespan,(double)_nTargetTimespan, bnOld.ToString(), bnNew.ToString());
    
    if (bnNew > UintToArith256(params.powLimit))
	{
        bnNew = UintToArith256(params.powLimit);
    }

    return bnNew.GetCompact();
}

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{
    // VIZ: GetNextWorkRequired returns the result of DeltaGravityWave
    return DeltaGravityWave(pindexLast, pblock->nTime, params);
}

unsigned int CalculateNextWorkRequired(const CBlockIndex* pindexLast, int64_t nFirstBlockTime, const Consensus::Params& params)
{
    if (params.fPowNoRetargeting)
        return pindexLast->nBits;

    // Limit adjustment step
    int64_t nActualTimespan = pindexLast->GetBlockTime() - nFirstBlockTime;
    if (nActualTimespan < params.nPowTargetTimespan/4)
        nActualTimespan = params.nPowTargetTimespan/4;
    if (nActualTimespan > params.nPowTargetTimespan*4)
        nActualTimespan = params.nPowTargetTimespan*4;

    // Retarget
    arith_uint256 bnNew;
    // arith_uint256 bnOld;
    bnNew.SetCompact(pindexLast->nBits);
    // bnOld = bnNew;

    // Viz: intermediate uint256 can overflow by 1 bit
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    bool fShift = bnNew.bits() > bnPowLimit.bits() - 1;
    if (fShift)
        bnNew >>= 1;
    bnNew *= nActualTimespan;
    bnNew /= params.nPowTargetTimespan;
    if (fShift)
        bnNew <<= 1;

    if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;

    return bnNew.GetCompact();
}

bool CheckProofOfWork(uint256 hash, unsigned int nBits, const Consensus::Params& params)
{
    bool fNegative;
    bool fOverflow;
    arith_uint256 bnTarget;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(params.powLimit))
        return false;

    // Check proof of work matches claimed amount
    if (UintToArith256(hash) > bnTarget)
        return false;

    return true;
}
