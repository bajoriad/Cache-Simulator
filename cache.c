/*
 * EECS 370, University of Michigan
 * Project 4: LC-2K Cache Simulator
 * Instructions are found in the project spec.
 */

#include <stdio.h>
#include <math.h>

#define MAX_CACHE_SIZE 256
#define MAX_BLOCK_SIZE 256

extern int mem_access(int addr, int write_flag, int write_data);
extern int get_num_mem_accesses();
int bitmasking(int size);

enum actionType
{
    cacheToProcessor,
    processorToCache,
    memoryToCache,
    cacheToMemory,
    cacheToNowhere
};

typedef struct blockStruct
{
    int data[MAX_BLOCK_SIZE];
    int dirty;
    int lruLabel;
    int set;
    int tag;
    int validbit; // added this
} blockStruct;

typedef struct cacheStruct
{
    blockStruct blocks[MAX_CACHE_SIZE];
    int blockSize;
    int numSets;
    int blocksPerSet;
    int blockoffsetbitsize; // block offset bits
    int setindexbitsize;    // set index bits
    int tagbitsize;         // tag size bits
} cacheStruct;

/* Global Cache variable */
cacheStruct cache;

void printAction(int, int, enum actionType);
void printCache();

/*
 * Set up the cache with given command line parameters. This is
 * called once in main(). You must implement this function.
 */
void cache_init(int blockSize, int numSets, int blocksPerSet)
{
    cache.blockSize = blockSize;
    cache.numSets = numSets;
    cache.blocksPerSet = blocksPerSet;
    cache.blockoffsetbitsize = log2(blockSize);
    cache.setindexbitsize = log2(numSets);
    cache.tagbitsize = 32 - cache.blockoffsetbitsize - cache.setindexbitsize;
    // initialising blocks with respective set numbers
    for (int i = 0; i < numSets; i++)
    {
        for (int j = (i * blocksPerSet); j < blocksPerSet; j++)
        {
            cache.blocks[j].dirty = 0;
            cache.blocks[j].validbit = 0;
            cache.blocks[j].lruLabel = 0;
            cache.blocks[j].tag = -1;
            cache.blocks[j].set = i;
        }
    }
}

/*
 * Access the cache. This is the main part of the project,
 * and should call printAction as is appropriate.
 * It should only call mem_access when absolutely necessary.
 * addr is a 16-bit LC2K word address.
 * write_flag is 0 for reads (fetch/lw) and 1 for writes (sw).
 * write_data is a word, and is only valid if write_flag is 1.
 * The return of mem_access is undefined if write_flag is 1.
 * Thus the return of cache_access is undefined if write_flag is 1.
 */
int cache_access(int addr, int write_flag, int write_data)
{
    // calculating the tag of the address
    printf("address = %d\n", addr);
    int address_tag = addr >> (cache.setindexbitsize + cache.blockoffsetbitsize);
    printf("address_tag = %d\n", address_tag);
    // calculating the set index of the address
    // int set_index_bit_mask = bitmasking(cache.setindexbitsize);
    int address_set_index = addr >> (cache.blockoffsetbitsize);
    address_set_index = address_set_index & bitmasking(cache.setindexbitsize);
    printf("address_set_index = %d\n", address_set_index);
    // calculating the block offset of t
    int address_block_offset = addr & bitmasking(cache.blockoffsetbitsize);
    // int address_block_offset = addr & block_offset_bit_mask;
    int found_tag_or_not = 0;
    int index_found_tag = -1; // index of the block with which the tag matched
    int index_when_not_found = -1;
    int max_lru = -1;
    int lru_corresponding_index = -1;
    // load instruction in cache
    if (write_flag == 0)
    {
        int start_search = address_set_index * cache.blocksPerSet;
        // checking if the tag of the address is mentioned in the given set
        for (int i = start_search; i < (start_search + cache.blocksPerSet); ++i)
        {
            if (cache.blocks[i].validbit == 1)
            {
                if (cache.blocks[i].tag == address_tag)
                {
                    found_tag_or_not = 1;
                    index_found_tag = i;
                }
            }
        }
        // if found_tag_or_not == 1 then it is a hit otherwise it is a miss
        // dealing with a load and hit condition first
        if (found_tag_or_not == 1)
        {
            printAction(addr, 1, cacheToProcessor);
            // updating the lru labels after finding a hit. The hit block has an lru of 0 and other
            // get incremented by 1. Only those blocks get updated that are not empty
            int lru = cache.blocks[index_found_tag].lruLabel;
            for (int i = start_search; i < (start_search + cache.blocksPerSet); ++i)
            {
                if (i == index_found_tag)
                {
                    cache.blocks[i].lruLabel = 0;
                }
                else
                {
                    if (cache.blocks[i].validbit == 1)
                    {
                        if (cache.blocks[i].lruLabel < lru)
                        {
                            cache.blocks[i].lruLabel = cache.blocks[i].lruLabel + 1;
                        }
                    }
                }
            }
            return cache.blocks[index_found_tag].data[address_block_offset];
        }
        else if (found_tag_or_not == 0)
        {
            // finding if there is an empty block in the set or not
            index_when_not_found = -1;
            for (int i = start_search; i < (start_search + cache.blocksPerSet); ++i)
            {
                if (cache.blocks[i].validbit == 0)
                {
                    index_when_not_found = i;
                    break;
                }
            }
            // if set full then finding the lru index
            if (index_when_not_found == -1)
            {
                for (int i = start_search; i < (start_search + cache.blocksPerSet); ++i)
                {
                    if (cache.blocks[i].lruLabel > max_lru)
                    {
                        max_lru = cache.blocks[i].lruLabel;
                        // recording index to  check diry bit or not
                        lru_corresponding_index = i;
                    }
                }
                index_when_not_found = lru_corresponding_index;
                int address = cache.blocks[index_when_not_found].tag << (cache.blockoffsetbitsize + cache.setindexbitsize);
                address = address + (cache.blocks[index_when_not_found].set << cache.blockoffsetbitsize);
                // printing appropriate commands
                if (cache.blocks[index_when_not_found].dirty == 1)
                {
                    int j = 0;
                    printAction(address, cache.blockSize, cacheToMemory);
                    for (int i = address; i < (address + cache.blockSize); ++i)
                    {
                        mem_access(i, 1, cache.blocks[index_when_not_found].data[j]);
                        ++j;
                    }
                }
                else if (cache.blocks[index_when_not_found].dirty == 0)
                {
                    printAction(address, cache.blockSize, cacheToNowhere);
                }
            }
            cache.blocks[index_when_not_found].validbit = 1;
            cache.blocks[index_when_not_found].lruLabel = -1;
            cache.blocks[index_when_not_found].dirty = 0;
            // updating the lru of the particular set
            for (int i = start_search; i < (start_search + cache.blocksPerSet); ++i)
            {
                cache.blocks[i].lruLabel = cache.blocks[i].lruLabel + 1;
            }
            // loading stuff in the cache from the memory
            int computation = addr % cache.blockSize;
            int start_address = addr - computation;
            cache.blocks[index_when_not_found].tag = start_address >> (cache.setindexbitsize + cache.blockoffsetbitsize);
            cache.blocks[index_when_not_found].set = (start_address >> (cache.blockoffsetbitsize)) & bitmasking(cache.setindexbitsize);
            for (int i = 0; i < cache.blockSize; ++i)
            {
                cache.blocks[index_when_not_found].data[i] = mem_access((start_address + i), 0, 0);
            }
            // printing put stuff
            printAction(start_address, cache.blockSize, memoryToCache);
            printAction(addr, 1, cacheToProcessor);
            return cache.blocks[index_when_not_found].data[address_block_offset];
        }
    }
    // store instruction in cache
    else if (write_flag == 1)
    {
        int start_search = address_set_index * cache.blocksPerSet;
        // checking if the tag of the address is mentioned in the given set
        for (int i = start_search; i < (start_search + cache.blocksPerSet); ++i)
        {
            if (cache.blocks[i].validbit == 1)
            {
                if (cache.blocks[i].tag == address_tag)
                {
                    found_tag_or_not = 1;
                    index_found_tag = i;
                }
            }
        }

        // store and hit condition
        if (found_tag_or_not == 1)
        {
            printAction(addr, 1, processorToCache);
            // updating the lru labels after finding a hit. The hit block has an lru of 0 and other
            // get incremented by 1. Only those blocks get updated that are not empty
            int lru = cache.blocks[index_found_tag].lruLabel;
            for (int i = start_search; i < (start_search + cache.blocksPerSet); ++i)
            {
                if (i == index_found_tag)
                {
                    cache.blocks[i].lruLabel = 0;
                    cache.blocks[i].dirty = 1;
                }
                else
                {
                    if (cache.blocks[i].validbit == 1)
                    {
                        if (cache.blocks[i].lruLabel < lru)
                        {
                            cache.blocks[i].lruLabel = cache.blocks[i].lruLabel + 1;
                        }
                    }
                }
            }
            cache.blocks[index_found_tag].data[address_block_offset] = write_data;
            cache.blocks[index_found_tag].dirty = 1;
            return 0;
        }
        // store and miss condition
        else if (found_tag_or_not == 0)
        {
            // finding if there is an empty block in the set or not
            index_when_not_found = -1;
            for (int i = start_search; i < (start_search + cache.blocksPerSet); ++i)
            {
                if (cache.blocks[i].validbit == 0)
                {
                    index_when_not_found = i;
                    break;
                }
            }
            // if set full then finding the lru index
            if (index_when_not_found == -1)
            {
                for (int i = start_search; i < (start_search + cache.blocksPerSet); ++i)
                {
                    if (cache.blocks[i].lruLabel > max_lru)
                    {
                        max_lru = cache.blocks[i].lruLabel;
                        // recording index to  check diry bit or not
                        lru_corresponding_index = i;
                    }
                }
                index_when_not_found = lru_corresponding_index;
                int address = cache.blocks[index_when_not_found].tag << (cache.blockoffsetbitsize + cache.setindexbitsize);
                address = address + (cache.blocks[index_when_not_found].set << cache.blockoffsetbitsize);
                if (cache.blocks[index_when_not_found].dirty == 1)
                {
                    int j = 0;
                    printAction(address, cache.blockSize, cacheToMemory);
                    for (int i = address; i < (address + cache.blockSize); ++i)
                    {
                        mem_access(i, 1, cache.blocks[index_when_not_found].data[j]);
                        ++j;
                    }
                }
                else if (cache.blocks[index_when_not_found].dirty == 0)
                {
                    printAction(address, cache.blockSize, cacheToNowhere);
                }
            }
            cache.blocks[index_when_not_found].validbit = 1;
            cache.blocks[index_when_not_found].lruLabel = -1;
            cache.blocks[index_when_not_found].dirty = 1;
            // updating the lru of the particular set
            for (int i = start_search; i < (start_search + cache.blocksPerSet); ++i)
            {
                cache.blocks[i].lruLabel = cache.blocks[i].lruLabel + 1;
            }
            // loading stuff in the cache from the memory
            int computation = addr % cache.blockSize;
            int start_address = addr - computation;
            cache.blocks[index_when_not_found].tag = start_address >> (cache.setindexbitsize + cache.blockoffsetbitsize);
            cache.blocks[index_when_not_found].set = (start_address >> (cache.blockoffsetbitsize)) & bitmasking(cache.setindexbitsize);
            for (int i = 0; i < cache.blockSize; ++i)
            {
                cache.blocks[index_when_not_found].data[i] = mem_access((start_address + i), 0, 0);
            }
            // printing put stuff
            printAction(start_address, cache.blockSize, memoryToCache);

            printAction(addr, 1, processorToCache);
            // if (addr == 6)
            // {
            //     printf("%s", "hello_world");
            //     return 0;
            // }
            cache.blocks[index_when_not_found].data[address_block_offset] = write_data;
            cache.blocks[index_when_not_found].dirty = 1;
            return 0;
        }
    }
}
// return mem_access(addr, write_flag, write_data);

// generating bit masks to extract tag, set index and block offset
// corner case : blocksize is 1 or no. of sets is 1
int bitmasking(int size)
{
    int bitmask = 0;
    for (int i = 0; i < size; ++i)
    {
        bitmask = bitmask << 1;
        bitmask = bitmask + 1;
    }
    return bitmask;
}

/*
 * print end of run statistics like in the spec. This is not required,
 * but is very helpful in debugging.
 * This should be called once a halt is reached.
 * DO NOT delete this function, or else it won't compile.
 * DO NOT print $$$ in this function
 */
void printStats()
{
    return;
}

/*
 * Log the specifics of each cache action.
 *
 * address is the starting word address of the range of data being transferred.
 * size is the size of the range of data being transferred.
 * type specifies the source and destination of the data being transferred.
 *  -    cacheToProcessor: reading data from the cache to the processor
 *  -    processorToCache: writing data from the processor to the cache
 *  -    memoryToCache: reading data from the memory to the cache
 *  -    cacheToMemory: evicting cache data and writing it to the memory
 *  -    cacheToNowhere: evicting cache data and throwing it away
 */
void printAction(int address, int size, enum actionType type)
{
    printf("$$$ transferring word [%d-%d] ", address, address + size - 1);

    if (type == cacheToProcessor)
    {
        printf("from the cache to the processor\n");
    }
    else if (type == processorToCache)
    {
        printf("from the processor to the cache\n");
    }
    else if (type == memoryToCache)
    {
        printf("from the memory to the cache\n");
    }
    else if (type == cacheToMemory)
    {
        printf("from the cache to the memory\n");
    }
    else if (type == cacheToNowhere)
    {
        printf("from the cache to nowhere\n");
    }
}

/*
 * Prints the cache based on the configurations of the struct
 * This is for debugging only and is not graded, so you may
 * modify it, but that is not recommended.
 */
void printCache()
{
    printf("\ncache:\n");
    for (int set = 0; set < cache.numSets; ++set)
    {
        printf("\tset %i:\n", set);
        for (int block = 0; block < cache.blocksPerSet; ++block)
        {
            printf("\t\t[ %i ]: {", block);
            for (int index = 0; index < cache.blockSize; ++index)
            {
                printf(" %i", cache.blocks[set * cache.blocksPerSet + block].data[index]);
            }
            printf(" }\n");
        }
    }
    printf("end cache\n");
}