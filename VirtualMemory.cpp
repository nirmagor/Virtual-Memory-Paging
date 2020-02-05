#include "VirtualMemory.h"
#include "PhysicalMemory.h"

struct dfsInformator{
    uint64_t& currFrame ;
    uint64_t& theChosenChild;
    uint64_t& justCreatedFrame;
    uint64_t p;
    uint64_t& pageSwapIn;
    uint64_t depth;
    uint64_t& max_dist;
    uint64_t& fatherFrame;
    uint64_t& iToDisconnect;
    uint64_t& maxFound;
    bool& toReturn;
    int& context;
    uint64_t& maxLeaf;
    uint64_t& maxPage;
    uint64_t& fatherOfMaxLeaf;

};






void dfs(dfsInformator& informator){

    informator.maxFound = informator.currFrame > informator.maxFound ? informator.currFrame :
                         informator .maxFound;
    if(informator.depth == TABLES_DEPTH){

        uint64_t calc = informator.pageSwapIn - informator.p;
        uint64_t absoluteVal = (calc < 0 )? (-1*calc ) : calc;
        uint64_t min = (NUM_PAGES - absoluteVal < absoluteVal)? NUM_PAGES - absoluteVal:
                       absoluteVal;
        informator.max_dist = (min > informator.max_dist)? min : informator.max_dist;
        informator.maxLeaf = informator.currFrame;
        informator.fatherOfMaxLeaf = informator.fatherFrame;
        informator.maxPage = informator.p;
        return;
    }
    uint64_t pToShift = informator.p<<OFFSET_WIDTH;
    uint64_t previousP = informator.p;
    bool isTableFree = true;
    for ( int i = 0 ; i < PAGE_SIZE ; ++ i )
    {
        word_t isZero;
        PMread (informator.currFrame*PAGE_SIZE +i, &isZero);
        if(isZero != 0 ){
            isTableFree = false;

            uint64_t saba = informator.fatherFrame;
            informator.fatherFrame = informator.currFrame;
            informator.currFrame = static_cast<uint64_t>(isZero);
            informator.depth++;
            informator.iToDisconnect = static_cast<uint64_t>(i);
            informator.p = pToShift + i;
            dfs (informator);
            if(informator.toReturn){
                return;
            }
            informator.p = previousP;
            informator.depth--;
            informator.currFrame = informator.fatherFrame;
            informator.fatherFrame = saba;
        }
    }
    if(isTableFree && informator.currFrame != informator.justCreatedFrame){

        informator.theChosenChild = informator.currFrame;
        informator.toReturn = true;
        informator.context = 1;
        PMwrite (informator.fatherFrame*PAGE_SIZE + informator.iToDisconnect, 0);
        return;

    }


}


uint64_t busOfOnes(uint64_t count){
    uint64_t i  = 0;
    uint64_t taker = 0LL;
    while(i < count){
        taker = (taker << 1) + 1;
        i++;
    }
    return taker;
}


uint64_t getLeftMostBits(uint64_t address ,uint64_t count){

    return address>>count;
//    uint64_t leftMostBits = (VIRTUAL_ADDRESS_WIDTH%OFFSET_WIDTH == 0) ? virtualAddress>> ( (cabillio-1)*OFFSET_WIDTH) : virtualAddress>>(cabillio*OFFSET_WIDTH);
}

void clearTable(uint64_t frameIndex) {
    for (uint64_t i = 0; i < PAGE_SIZE; ++i) {
        PMwrite(frameIndex * PAGE_SIZE + i, 0);
    }
}

void VMinitialize() {
    clearTable(0);
}



void mainAlgorithm(uint64_t virtualAddress, word_t value, word_t* toRead, int readOrWrite){
    uint64_t leftMostBits;
    word_t valueX;
    uint64_t weWork = virtualAddress;
    uint64_t currFrame = 0;
    bool doRestore = false;

    int cabillio = VIRTUAL_ADDRESS_WIDTH / OFFSET_WIDTH; // 5

    uint64_t pageSwapIn = virtualAddress>>OFFSET_WIDTH;



    int rounds = (VIRTUAL_ADDRESS_WIDTH%OFFSET_WIDTH == 0) ? cabillio : cabillio+1;

    for(int lap = rounds;  0 < lap; lap--){

        uint64_t shift = static_cast<uint64_t>(( lap - 1) * OFFSET_WIDTH);
        leftMostBits = getLeftMostBits ( weWork, shift );// left most 4

        PMread (currFrame*PAGE_SIZE + leftMostBits, &valueX);
        if(lap == 1){
            if(doRestore)
            {
                PMrestore ( currFrame , pageSwapIn  );
            }
            if(readOrWrite == 0){
                PMread (currFrame*PAGE_SIZE + leftMostBits, toRead);
            }
            else
            {
                PMwrite ( currFrame * PAGE_SIZE + leftMostBits , value );
            }
            return ;
        }
        if(valueX == 0){

            uint64_t rootFrame = 0;
            uint64_t theChosenChild = 0;
            uint64_t justCreatedFrame = currFrame;
            uint64_t p = 0;
            uint64_t depth = 0;
            uint64_t max_dist = 0;
            uint64_t fatherFrame = 0;
            uint64_t iToDisconnect = 0;
            uint64_t maxFound = 0;
            bool toReturn = false;
            int context = 0;
            uint64_t maxLeaf = 0;
            uint64_t maxPage = 0;
            uint64_t fatherOfMaxLeaf = 0;

            struct dfsInformator informator = {rootFrame,
                                               theChosenChild,
                                               justCreatedFrame,
                                               p,
                                               pageSwapIn,
                                               depth,
                                               max_dist,
                                               fatherFrame,
                                               iToDisconnect,
                                               maxFound ,
                                               toReturn,
                                               context,
                                               maxLeaf,
                                               maxPage,
                                               fatherOfMaxLeaf};
            dfs(informator);
            if(informator.context != 1)
            {
                if ( informator . maxFound + 1 < NUM_FRAMES )
                {

                    informator . theChosenChild = informator . maxFound + 1;
                    informator . context = 2;
                }
                else
                {
                    informator . context = 3;
                    for ( int i = 0 ; i < PAGE_SIZE ; i ++ )
                    {
                        word_t comperator;
                        PMread ( informator . fatherOfMaxLeaf * PAGE_SIZE + i , & comperator );
                        if ( static_cast<uint64_t> (comperator) == informator . maxLeaf )
                        {
                            PMwrite ( informator . fatherOfMaxLeaf * PAGE_SIZE + i , 0 );
                        }
                        informator . theChosenChild = informator . maxLeaf;
                    }
                    PMevict ( informator . theChosenChild , informator . maxPage );
                }
            }
            PMwrite (currFrame*PAGE_SIZE +leftMostBits, static_cast<word_t>(informator.theChosenChild));
            if(lap != 2)
            {
                clearTable (informator.theChosenChild);
            }
            if(lap == 2){

                doRestore = true;
            }
            valueX = static_cast<word_t>(informator.theChosenChild);
        }
        currFrame = static_cast<uint64_t>(valueX);
        weWork = weWork & busOfOnes (shift);
    }
}

int VMread(uint64_t virtualAddress, word_t* value) {


    if(virtualAddress>>VIRTUAL_ADDRESS_WIDTH != 0){
        return 0;
    }

    mainAlgorithm (virtualAddress, 0, value,0);
    return 1;
}


int VMwrite(uint64_t virtualAddress, word_t value) {
    if(virtualAddress>>VIRTUAL_ADDRESS_WIDTH != 0){
        return 0;
    }
    mainAlgorithm (virtualAddress, value, nullptr,1);
    return 1;
}
