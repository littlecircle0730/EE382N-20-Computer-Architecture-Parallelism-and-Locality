/*
 * Copyright 2002-2020 Intel Corporation.
 * 
 * This software is provided to you as Sample Source Code as defined in the accompanying
 * End User License Agreement for the Intel(R) Software Development Products ("Agreement")
 * section 1.L.
 * 
 * This software and the related documents are provided as is, with no express or implied
 * warranties, other than those that are expressly stated in the License.
 */

/*! @file
 *  This file contains an ISA-portable cache simulator
 *  data cache hierarchies
 */


#include "pin.H"

#include <iostream>
#include <fstream>

#include "dcache.H"
#include "pin_profile.H"
using std::ostringstream;
using std::string;
using std::cerr;
using std::endl;

std::ofstream outFile;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE,    "pintool",
    "o", "dcache.out", "specify dcache file name");
KNOB<BOOL>   KnobTrackLoads(KNOB_MODE_WRITEONCE,    "pintool",
    "tl", "0", "track individual loads -- increases profiling time");
KNOB<BOOL>   KnobTrackStores(KNOB_MODE_WRITEONCE,   "pintool",
   "ts", "0", "track individual stores -- increases profiling time");
KNOB<UINT32> KnobThresholdHit(KNOB_MODE_WRITEONCE , "pintool",
   "rh", "100", "only report memops with hit count above threshold");
KNOB<UINT32> KnobThresholdMiss(KNOB_MODE_WRITEONCE, "pintool",
   "rm","100", "only report memops with miss count above threshold");
KNOB<UINT32> KnobCacheSize(KNOB_MODE_WRITEONCE, "pintool",
    "c","32", "cache size in kilobytes");
KNOB<UINT32> KnobLineSize(KNOB_MODE_WRITEONCE, "pintool",
    "b","32", "cache block size in bytes");
KNOB<UINT32> KnobAssociativity(KNOB_MODE_WRITEONCE, "pintool",
    "a","4", "cache associativity (1 for direct mapped)");

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr <<
        "This tool represents a cache simulator.\n"
        "\n";

    cerr << KNOB_BASE::StringKnobSummary() << endl; 
    return -1;
}

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

// wrap configuation constants into their own name space to avoid name clashes
namespace DL1
{
    const UINT32 max_sets = KILO; // cacheSize / (lineSize * associativity);
    const UINT32 max_associativity = 256; // associativity;
    const CACHE_ALLOC::STORE_ALLOCATION allocation = CACHE_ALLOC::STORE_ALLOCATE;

    typedef CACHE_LEAST_RECENTLY_USED(max_sets, max_associativity, allocation) CACHE;
}

namespace DL2
{
    const UINT32 max_sets = 32 * KILO; // cacheSize / (lineSize * associativity);
    const UINT32 max_associativity = 256; // associativity;
    const CACHE_ALLOC::STORE_ALLOCATION allocation = CACHE_ALLOC::STORE_ALLOCATE;

    typedef CACHE_LEAST_RECENTLY_USED(max_sets, max_associativity, allocation) CACHE;
}

DL1::CACHE* dl1 = NULL;

DL2::CACHE* dl2 = NULL;

typedef enum
{
    COUNTER_MISS = 0,
    COUNTER_HIT = 1,
    COUNTER_NUM
} COUNTER;

ADDRINT * dirty_addr_arr = (ADDRINT*)(malloc(2*sizeof(ADDRINT)));
UINT32 * write_back_arr = (UINT32*)(malloc(2*sizeof(UINT32)));


typedef  COUNTER_ARRAY<UINT64, COUNTER_NUM> COUNTER_HIT_MISS;


// holds the counters with misses and hits
// conceptually this is an array indexed by instruction address
COMPRESSOR_COUNTER<ADDRINT, UINT32, COUNTER_HIT_MISS> profile;

/* ===================================================================== */

VOID LoadMulti(ADDRINT addr, UINT32 size, UINT32 instId)
{
    // first level D-cache
    ADDRINT *dirty_addr = {};
    UINT32 *write_back = {};
    UINT32 arr_size;
    const BOOL dl1Hit = dl1->Access(addr, size, CACHE_BASE::ACCESS_TYPE_LOAD, dirty_addr, write_back, arr_size);

    const COUNTER counter = dl1Hit ? COUNTER_HIT : COUNTER_MISS;
	std::cout << "Profile!\n";
	profile[instId][counter]++;
}

/* ===================================================================== */

VOID StoreMulti(ADDRINT addr, UINT32 size, UINT32 instId)
{
    // first level D-cache
    ADDRINT *dirty_addr = {};
    UINT32 *write_back = {};
    UINT32 arr_size;
    const BOOL dl1Hit = dl1->Access(addr, size, CACHE_BASE::ACCESS_TYPE_STORE, dirty_addr, write_back, arr_size);

    const COUNTER counter = dl1Hit ? COUNTER_HIT : COUNTER_MISS;
	std::cout << "Profile!\n";
    profile[instId][counter]++;
}

/* ===================================================================== */

VOID LoadSingle(ADDRINT addr, UINT32 instId)
{
    // @todo we may access several cache lines for 
    // first level D-cache
    ADDRINT dirty_addr = 0xFFFFFFFF;
    UINT32 write_back;
    const BOOL dl1Hit = dl1->AccessSingleLine(addr, CACHE_BASE::ACCESS_TYPE_LOAD, dirty_addr, write_back);

    const COUNTER counter = dl1Hit ? COUNTER_HIT : COUNTER_MISS;
	std::cout << "Profile!\n";
    profile[instId][counter]++;
}
/* ===================================================================== */

VOID StoreSingle(ADDRINT addr, UINT32 instId)
{
    // @todo we may access several cache lines for 
    // first level D-cache
    ADDRINT dirty_addr = 0xFFFFFFFF;
    UINT32 write_back;
    const BOOL dl1Hit = dl1->AccessSingleLine(addr, CACHE_BASE::ACCESS_TYPE_STORE, dirty_addr, write_back);

    const COUNTER counter = dl1Hit ? COUNTER_HIT : COUNTER_MISS;
	std::cout << "Profile!\n";
    profile[instId][counter]++;
}

/* ===================================================================== */

VOID LoadMultiFast(ADDRINT addr, UINT32 size)
{
	//std::cout << "LOAD MULTI FAST\n";
    //ADDRINT * dirty_addr_arr;// = (ADDRINT*)(malloc(size));    
	ADDRINT adr;
    //UINT32 * write_back_arr;// = (UINT32*)(malloc(size));
    UINT32 wb, arr_size;
	//dirty_addr_arr = (ADDRINT*) malloc(2*sizeof(ADDRINT));
	//write_back_arr = (UINT32*) malloc(2*sizeof(UINT32));
	ADDRINT* dirty_addr_tmp = dirty_addr_arr;
	UINT32* write_back_tmp = write_back_arr;
    const BOOL dl1Hit = dl1->Access(addr, size, CACHE_BASE::ACCESS_TYPE_LOAD, dirty_addr_arr, write_back_arr, arr_size);
	if (!dl1Hit)
	{
		for (UINT32 i = 0; i < arr_size; i++) 
		{
			adr = 0xFFFFFFFF;
			if (*write_back_arr) 
			{ 
				//std::cout << "	Write Back to L2...\n";
				dl2->AccessSingleLine(*dirty_addr_arr, CACHE_BASE::ACCESS_TYPE_LOAD, adr, wb); 
			}
			write_back_arr ++;
			dirty_addr_arr ++;
		}
		dl2->Access(addr, size, CACHE_BASE::ACCESS_TYPE_LOAD, dirty_addr_arr, write_back_arr, arr_size);
	}
	//free (dirty_addr_arr);
	//free (write_back_arr);
	dirty_addr_arr = dirty_addr_tmp;
	write_back_arr = write_back_tmp;
	//std::cout << "END LOAD MULTI FAST\n\n\n";
}

/* ===================================================================== */

VOID StoreMultiFast(ADDRINT addr, UINT32 size)
{
	//std::cout << "STORE MULTI FAST\n";
    //ADDRINT * dirty_addr_arr;// = (ADDRINT*)(malloc(size));
    ADDRINT adr;
    //UINT32 * write_back_arr;// = (UINT32*)(malloc(size));
    UINT32 wb, arr_size;
	//dirty_addr_arr = (ADDRINT*) malloc(2*sizeof(ADDRINT));
	//write_back_arr = (UINT32*) malloc(2*sizeof(UINT32));
	ADDRINT* dirty_addr_tmp = dirty_addr_arr;
	UINT32* write_back_tmp = write_back_arr;
    const BOOL dl1Hit = dl1->Access(addr, size, CACHE_BASE::ACCESS_TYPE_STORE, dirty_addr_arr, write_back_arr, arr_size);
	if (!dl1Hit) 
	{
		for (UINT32 i = 0; i < arr_size; i++) 
		{
			adr = 0xFFFFFFFF;
			if (*write_back_arr) 
			{ 
				//std::cout << "	Write Back to L2...\n";
				dl2->AccessSingleLine(*dirty_addr_arr, CACHE_BASE::ACCESS_TYPE_STORE, adr, wb); 
			}
			write_back_arr ++;
			dirty_addr_arr ++;
		}
		dl2->Access(addr, size, CACHE_BASE::ACCESS_TYPE_STORE, dirty_addr_arr, write_back_arr, arr_size);
	}
	//free (dirty_addr_arr);
	//free (write_back_arr);
	dirty_addr_arr = dirty_addr_tmp;
	write_back_arr = write_back_tmp;
	//std::cout << "END STORE MULTI FAST\n\n\n";
}

/* ===================================================================== */

VOID LoadSingleFast(ADDRINT addr)
{
	//std::cout << "LOAD SINGLE FAST\n";
    ADDRINT dirty_addr = 0x0;
    ADDRINT adr = 0xFFFFFFFF;
    UINT32 write_back, wb;
    const BOOL dl1Hit = dl1->AccessSingleLine(addr, CACHE_BASE::ACCESS_TYPE_LOAD, dirty_addr, write_back);    
	if (!dl1Hit)
	{
		// Write back to L2 from L1 since L1 has a load miss
		// MAYBE CHANGE STORE -> LOAD?
		if (write_back) 
		{ 
			//std::cout << "	Write Back to L2...\n";
			dl2->AccessSingleLine(dirty_addr, CACHE_BASE::ACCESS_TYPE_LOAD, adr, wb); 
		}
		dl2->AccessSingleLine(addr, CACHE_BASE::ACCESS_TYPE_LOAD, dirty_addr, write_back);
	}
	//std::cout << "END LOAD SINGLE FAST\n\n\n";
}

/* ===================================================================== */

VOID StoreSingleFast(ADDRINT addr)
{
	//std::cout << "STORE SINGLE FAST\n";
    ADDRINT dirty_addr = 0x0;
    ADDRINT adr = 0xFFFFFFFF;
    UINT32 write_back, wb;
    // If it's a L1 miss, then L1 will already be replaced in here but L2 does not yet.
    const BOOL dl1Hit = dl1->AccessSingleLine(addr, CACHE_BASE::ACCESS_TYPE_STORE, dirty_addr, write_back);    
    if (!dl1Hit) 
    {
        if (write_back) 
		{ 
			//std::cout << "	Write Back to L2...\n";
			dl2->AccessSingleLine(dirty_addr, CACHE_BASE::ACCESS_TYPE_STORE, adr, wb);
		}
        dl2->AccessSingleLine(addr, CACHE_BASE::ACCESS_TYPE_STORE, dirty_addr, write_back);
    }
	//std::cout << "END STORE SINGLE FAST\n\n\n";
}



/* ===================================================================== */

VOID Instruction(INS ins, void * v)
{
    if (INS_IsMemoryRead(ins) && INS_IsStandardMemop(ins))
    {
        // map sparse INS addresses to dense IDs
        const ADDRINT iaddr = INS_Address(ins);
        const UINT32 instId = profile.Map(iaddr);

        const UINT32 size = INS_MemoryReadSize(ins);
        const BOOL   single = (size <= 4);
                
        if( KnobTrackLoads )
        {
            if( single )
            {
                INS_InsertPredicatedCall(
                    ins, IPOINT_BEFORE, (AFUNPTR) LoadSingle,
                    IARG_MEMORYREAD_EA,
                    IARG_UINT32, instId,
                    IARG_END);
            }
            else
            {
                INS_InsertPredicatedCall(
                    ins, IPOINT_BEFORE,  (AFUNPTR) LoadMulti,
                    IARG_MEMORYREAD_EA,
                    IARG_MEMORYREAD_SIZE,
                    IARG_UINT32, instId,
                    IARG_END);
            }
                
        }
        else
        {
            if( single )
            {
                INS_InsertPredicatedCall(
                    ins, IPOINT_BEFORE,  (AFUNPTR) LoadSingleFast,
                    IARG_MEMORYREAD_EA,
                    IARG_END);
                        
            }
            else
            {
                INS_InsertPredicatedCall(
                    ins, IPOINT_BEFORE,  (AFUNPTR) LoadMultiFast,
                    IARG_MEMORYREAD_EA,
                    IARG_MEMORYREAD_SIZE,
                    IARG_END);
            }
        }
    }
        
    if ( INS_IsMemoryWrite(ins) && INS_IsStandardMemop(ins))
    {
        // map sparse INS addresses to dense IDs
        const ADDRINT iaddr = INS_Address(ins);
        const UINT32 instId = profile.Map(iaddr);
            
        const UINT32 size = INS_MemoryWriteSize(ins);

        const BOOL   single = (size <= 4);
                
        if( KnobTrackStores )
        {
            if( single )
            {
                INS_InsertPredicatedCall(
                    ins, IPOINT_BEFORE,  (AFUNPTR) StoreSingle,
                    IARG_MEMORYWRITE_EA,
                    IARG_UINT32, instId,
                    IARG_END);
            }
            else
            {
                INS_InsertPredicatedCall(
                    ins, IPOINT_BEFORE,  (AFUNPTR) StoreMulti,
                    IARG_MEMORYWRITE_EA,
                    IARG_MEMORYWRITE_SIZE,
                    IARG_UINT32, instId,
                    IARG_END);
            }
                
        }
        else
        {
            if( single )
            {
                INS_InsertPredicatedCall(
                    ins, IPOINT_BEFORE,  (AFUNPTR) StoreSingleFast,
                    IARG_MEMORYWRITE_EA,
                    IARG_END);
                        
            }
            else
            {
                INS_InsertPredicatedCall(
                    ins, IPOINT_BEFORE,  (AFUNPTR) StoreMultiFast,
                    IARG_MEMORYWRITE_EA,
                    IARG_MEMORYWRITE_SIZE,
                    IARG_END);
            }
        }
            
    }
}

/* ===================================================================== */

VOID Fini(int code, VOID * v)
{
    // print D-cache profile
    // @todo what does this print
    
    outFile << "PIN:MEMLATENCIES 1.0. 0x0\n";
            
    outFile <<
        "#\n"
        "# DCACHE L1 stats\n"
        "#\n";
    
    outFile << dl1->StatsLong("# ", CACHE_BASE::CACHE_TYPE_DCACHE);
    
    outFile <<
        "#\n"
        "# DCACHE L2 stats\n"
        "#\n";
    outFile << dl2->StatsLong("# ", CACHE_BASE::CACHE_TYPE_DCACHE);

    if( KnobTrackLoads || KnobTrackStores ) {
        outFile <<
            "#\n"
            "# LOAD stats\n"
            "#\n";
        
        outFile << profile.StringLong();
    }
    outFile.close();
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char *argv[])
{
    PIN_InitSymbols();

    if( PIN_Init(argc,argv) )
    {
        return Usage();
    }

    outFile.open(KnobOutputFile.Value().c_str());

    dl1 = new DL1::CACHE("L1 Data Cache", 
                         32 * KILO,
						 32, 4);
                         //KnobCacheSize.Value() * KILO,
						 //KnobLineSize.Value(),
                         //KnobAssociativity.Value());

    dl2 = new DL2::CACHE("L2 Data Cache", 
                         2048 * KILO,
					 	 64, 16);
                         //KnobCacheSize.Value() * KILO,
                         //KnobLineSize.Value(),
                         //KnobAssociativity.Value());
    
    profile.SetKeyName("iaddr          ");
    profile.SetCounterName("dcache:miss        dcache:hit");

    COUNTER_HIT_MISS threshold;

    threshold[COUNTER_HIT] = KnobThresholdHit.Value();
    threshold[COUNTER_MISS] = KnobThresholdMiss.Value();
    
    profile.SetThreshold( threshold );
    
    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns

    PIN_StartProgram();
    
    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
