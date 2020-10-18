#include<bits/stdc++.h>
#include <chrono> 
using namespace std;
using namespace std::chrono; 
#define perfIteration 25

struct depthInfo{
    int index, depth;

    depthInfo(){
        index = 0, depth = 0;
    }
};

struct linkInfo{
    int linkIndex, linkValue;

    linkInfo(){
        linkIndex = 0, linkValue = 0;
    }
};

bool compareDepth(depthInfo i1, depthInfo i2)
{
    return (i1.depth < i2.depth);
}


bool IsParenStringBalanced(string parenString){

    stack<char> S;
    int sz = parenString.size();

    for(int i=0; i<sz; i++){

        char currentChar = parenString[i];

        if(currentChar == '['){
            S.push(currentChar);
        }
        else{

            if(currentChar != ']'){
                return false;
            }
            else{
              S.pop();
            }
        }
    }

    return S.empty();
}

int main(int argc, char** argv){


   if (argc < 2) {
        std::cerr << "Usage: process <filename> [filename...]\n";
        return EXIT_FAILURE;
    }

    for (int i=1; i<argc; i++) {

        long long int average_time = 0;
        int len, finalOutputArraySize;
        int* resultArr;

        string inputString;
        std::ifstream infile(argv[i]);
        infile>>inputString;

        bool isStringValid = IsParenStringBalanced(inputString);

        if(isStringValid == false){
            cout<<"Given string not balanced, please enter proper paren string"<<endl;
            return 0;
        }

        for(int k=0; k< perfIteration; k++)
        {

        // Get starting timepoint 
            auto start = high_resolution_clock::now(); 
           
            len = inputString.size();
            int totalNodes = 0;

            int *nodePrefixSum = new int[len];
            int *depthPrefixSum = new int[len];

            for(int i=0; i< len; i++){
                nodePrefixSum[i]  = 0;
                depthPrefixSum[i] = 0;
            }
            // first pass: count nodes & computing nesting depth
            for(int i=1; i< len; i++){

                if(inputString[i-1] == '['){
                    nodePrefixSum[i]= nodePrefixSum[i-1]  +  1;
                    depthPrefixSum[i]= depthPrefixSum[i-1] + 1;
                    totalNodes++;
                }
                else{
                    nodePrefixSum[i]= nodePrefixSum[i-1];
                    depthPrefixSum[i]= depthPrefixSum[i-1] + -1;
                }
            }

            finalOutputArraySize = totalNodes* 2 - 1;
            int* node = new int[totalNodes];
            int* depth  = new int[totalNodes];
            int* bfsOrder= new int[totalNodes];
            int* parent = new int[totalNodes];
            int* childrenCount = new int[totalNodes];
            int* nChild = new int[totalNodes];
            int* alloc = new int[totalNodes];
            resultArr= new int[finalOutputArraySize];
            // second pass: reducing to nodes
            for(int i=0; i< len; i++){
                depth[ nodePrefixSum[i] ] = depthPrefixSum[i];
            }

            // third pass: sort by depth
            vector< depthInfo > depthInfoVector;

            for(int i=0; i< totalNodes; i++){
                depthInfo info;
                info.index = i;
                info.depth = depth[i];
                depthInfoVector.push_back(info);
            }

            sort(depthInfoVector.begin(), depthInfoVector.end(), compareDepth);

            int nodeLabel = 0;

            for(int i=0; i< totalNodes; i++){
                bfsOrder[ depthInfoVector[i].index ] = nodeLabel;
                nodeLabel++;
                parent[i] = -1;
                childrenCount[i] = -1;
                nChild[i] = 0;
                alloc[i] = 0;
            }

            for(int i=0; i< finalOutputArraySize; i++){
                resultArr[i] = 0;
            }
            // pass: first parent generation
            vector<linkInfo> linkInfoVector;
            int linkCount = 0;

            for(int i=0; i< totalNodes; i++){

                if( depth[i+1] == depth[i] + 1){
                    parent[ bfsOrder[i + 1] ] = bfsOrder[i];

                    linkInfo info;
                    info.linkIndex = bfsOrder[i+1];
                    info.linkValue = bfsOrder[i];
                    linkInfoVector.push_back(info);
                    linkCount++;
                }
            }
            // pass: propagate parent link
             int childCount;

             for(int i=0; i< linkCount - 1 ; i++){

                int curIndex = linkInfoVector[i].linkIndex + 1;
                int curLinkValue = linkInfoVector[i].linkValue;
                int nextIndex = linkInfoVector[i + 1].linkIndex;
                childCount = 1;
                childrenCount[curIndex - 1] = childCount;

                while(curIndex < nextIndex){
                    parent[curIndex] = curLinkValue;
                    childrenCount[curIndex] = ++childCount;
                    curIndex++;
                }
             }
             // process last link
             int linkIter = linkInfoVector[ linkCount - 1 ].linkIndex + 1;
             int lastLinkValue = linkInfoVector[linkCount - 1].linkValue;
             childCount = 1;
             childrenCount[linkIter - 1] = childCount;

             while(linkIter < totalNodes){
                parent[linkIter] = lastLinkValue;
                childrenCount[linkIter] = ++childCount;
                linkIter++;
             }

             // pass: Scatter child count
             for(int i=0; i< totalNodes; i++){

                if(parent[i] != parent[i+1]){
                    nChild[parent[i]] = childrenCount[i];
                }
             }
             // pass: Allocate

             for(int i=1; i<totalNodes; i++){
                alloc[i] = alloc[i-1] + nChild[i-1] + 1;
             }

             //last pass: generate result
            int iterResult= 0;
            int iterAlloc = 0;

             for(int i=0; i< totalNodes; i++){

                resultArr[iterResult++] = nChild[i];

                for(int j=0; j< nChild[i]; j++){
                    resultArr[iterResult++] = alloc[ iterAlloc++ + 1 ];
                }
             }

            auto stop = high_resolution_clock::now(); 
            auto duration = duration_cast<microseconds>(stop - start); 
            average_time+= duration.count(); 
        }


            cout<<endl;
            cout<<"-------------------------------------------------------------------------------------";
            cout<< "Input file name: "<< argv[i]<<" Input String length: "<< len <<endl;
            cout<<"Time taken by sequential version: "<< average_time/perfIteration << " microseconds" << endl; 
            cout<<"-------------------------------------------------------------------------------------";
            
            /*
              cout<<"resultArray: ";
              for(int i=0; i< finalOutputArraySize; i++){
                    cout<<resultArr[i]<<" ";
                }
                cout<<endl; 
            */

    }

    return 0;
}
