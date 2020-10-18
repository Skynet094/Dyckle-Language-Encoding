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
 

// begin: parallelizing prefix sum / scan
struct prefixSumArr{

    int* arr; 
    int const1;

    prefixSumArr(){
        const1  = 0; 
        arr = NULL;
    }
}; 

//global variables: for pthreads
string inputString;
int len; 

void* prefixSumRunner(void* arg)
{
    prefixSumArr* prefixSum = (prefixSumArr*) arg;
    int* sumArr = prefixSum->arr;
    int const1;
    const1 = prefixSum->const1; 

     for(int i=1; i< len; i++){

        if(inputString[i-1] == '['){
            sumArr[i]= sumArr[i-1]  +  1; 
        }
        else{
            sumArr[i]= sumArr[i-1] + const1; 
        }
    }

    pthread_exit(0);
}
// end: parallelizing prefix sum / scan



// start: parallelizing sort operation
// array of size MAX  

#define THREAD_MAX 4
#define MAX_NODES 10000000
depthInfo depthInfoVector[MAX_NODES]; 

// thread control parameters
struct task {
    int tsk_no;
    int tsk_low;
    int tsk_high;
};

  
 // merge function for merging two parts
void merge(int low, int mid, int high)
{

    // n1 is size of left part and n2 is size of right part
    int n1 = mid - low + 1;
    int n2 = high - mid;

    depthInfo *left = (depthInfo*)malloc(n1 * sizeof(depthInfo));
    depthInfo *right = (depthInfo*)malloc(n2 * sizeof(depthInfo));

    int i;
    int j;

    // storing values in left part
    for (i = 0; i < n1; i++)
        left[i] = depthInfoVector[i + low];

    // storing values in right part
    for (i = 0; i < n2; i++)
        right[i] = depthInfoVector[i + mid + 1];

    int k = low;

    i = j = 0;

    // merge left and right in ascending order
    while (i < n1 && j < n2) {
        if (left[i].depth <= right[j].depth)
            depthInfoVector[k++] = left[i++];
        else
            depthInfoVector[k++] = right[j++];
    }

    // insert remaining values from left
    while (i < n1)
        depthInfoVector[k++] = left[i++];

    // insert remaining values from right
    while (j < n2)
        depthInfoVector[k++] = right[j++];

    free(left);
    free(right);
}

// merge sort function
void mergeSort(int low, int high)
{

    // calculating mid point of array
    int mid = low + (high - low) / 2;

    if (low < high) {
        // calling first half
        mergeSort(low, mid);

        // calling second half
        mergeSort(mid + 1, high);

        // merging the two halves
        merge(low, mid, high);
    }
}

// thread function for multi-threading
void *threadedMergeSort(void *arg)
{
    task *tsk = (task*)arg;
    int low;
    int high;

    // calculating low and high
    low = tsk->tsk_low;
    high = tsk->tsk_high;

    // evaluating mid point
    int mid = low + (high - low) / 2;

    if (low < high) {
        mergeSort(low, mid);
        mergeSort(mid + 1, high);
        merge(low, mid, high);
    }

    return 0;
}
//end: parallelizing sort operation


//begin: parallelizing: propagating parent link

   linkInfo linkInfoVector[MAX_NODES];
   int childrenCount[MAX_NODES];
   int parent[MAX_NODES];

   void *threadedLinkPropagate(void *arg)
   {
	    task *tsk = (task*)arg;
	    int start = tsk->tsk_low;;
	    int end = tsk->tsk_high;

	    int childCount = 1;
	    //cout<<"inside thread: "<< start<<" "<< end<<endl;

        for(int i=start; i<= end ; i++){

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


	    pthread_exit(0);
	}

//end


int main(int argc, char** argv){


   if (argc < 2) {
        std::cerr << "Usage: process <filename> [filename...]\n";
        return EXIT_FAILURE;
    }

     struct task *tsk;

    for (int i=1; i<argc; i++) {

        std::ifstream infile(argv[i]);
        infile>>inputString;

        bool isStringValid = IsParenStringBalanced(inputString);

        if(isStringValid == false){
            cout<<"Given string not balanced, please enter proper paren string"<<endl;
            return 0;
        }

      long long int average_time = 0;
      long long int block1_avg_time = 0;
      long long int block2_avg_time = 0;
      long long int block3_avg_time = 0;
	  long long int block4_avg_time = 0;

	  long long int sub_block4_1_avg_time = 0;
	  long long int sub_block4_2_avg_time = 0;
	  long long int sub_block4_3_avg_time = 0;
	  long long int sub_block4_4_avg_time = 0;

      int finalOutputArraySize;
      int* resultArr, totalNodes;


     len = inputString.size();
     totalNodes = 0;

  	 prefixSumArr nodePrefixSum, depthPrefixSum;
     nodePrefixSum.arr = new int[len];
     depthPrefixSum.arr = new int[len];

     nodePrefixSum.const1 = 0; 
     depthPrefixSum.const1= -1;

     for(int i=0; i< len; i++){

        if(inputString[i] == '['){
            totalNodes++;
        }

        nodePrefixSum.arr[i]  = 0;
        depthPrefixSum.arr[i] = 0;
     } 

    finalOutputArraySize;	    

     for(int k=0; k< perfIteration; k++){

        // Get starting timepoint 
            auto start = high_resolution_clock::now(); 

            // first pass: count nodes & computing nesting depth   
		    // block_1_start: memory allocations
            // parallel computation of prefix sums
            pthread_t tids[2]; 
            pthread_attr_t attr[2];
          
            pthread_attr_init(&attr[0]);
            pthread_create(&tids[0], &attr[0], prefixSumRunner, &nodePrefixSum); 

         
            pthread_attr_init(&attr[1]);
            pthread_create(&tids[1], &attr[1], prefixSumRunner, &depthPrefixSum);

            auto stop1 = high_resolution_clock::now(); 
            auto duration1 = duration_cast<microseconds>(stop1 - start);
            block1_avg_time+= duration1.count(); 
            // block_1_end

			int* node= new int[len];
			int* depth= new int[len];
			int* bfsOrder= new int[totalNodes];
			int* nChild= new int[totalNodes];
			int* alloc= new int[totalNodes];
            // block_2_start
            auto start2 = high_resolution_clock::now();

            for (int i = 0; i < 2; i++) {
                    pthread_join(tids[i], NULL); 
                } 
            // second pass: reducing to nodes
            for(int i=0; i< len; i++){
                depth[ nodePrefixSum.arr[i] ] = depthPrefixSum.arr[i];
            }
                

            // third pass: sort by depth 

            for(int i=0; i< totalNodes; i++){
                depthInfo info;
                info.index = i;
                info.depth = depth[i];
                depthInfoVector[i] = info;
            }

            auto stop2 = high_resolution_clock::now(); 
            auto duration2 = duration_cast<microseconds>(stop2 - start2);
            block2_avg_time+= duration2.count(); 
            //block_2_end

            //sort(depthInfoVector.begin(), depthInfoVector.end(), compareDepth);

            /* seems unnecessary
            for(int i=0; i< finalOutputArraySize; i++){
                resultArr[i] = 0;
            }
			*/

           // block_3_start
           auto start3 = high_resolution_clock::now();

           pthread_t threads[THREAD_MAX];
           struct task tsklist[THREAD_MAX];

           int sortLen = totalNodes / THREAD_MAX;

            int low = 0;

            for (int i = 0; i < THREAD_MAX; i++, low += sortLen) {
                tsk = &tsklist[i];
                tsk->tsk_no = i;

                tsk->tsk_low = low;
                tsk->tsk_high = low + sortLen - 1;
                if (i == (THREAD_MAX - 1))
                    tsk->tsk_high = totalNodes - 1; 
 
            }

            // creating 4 threads
            for (int i = 0; i < THREAD_MAX; i++) {
                tsk = &tsklist[i];
                pthread_create(&threads[i], NULL, threadedMergeSort, tsk);
            }

            // joining all 4 threads
            for (int i = 0; i < THREAD_MAX; i++)
                pthread_join(threads[i], NULL);


            // merging the final 4 parts 
            struct task *tskm = &tsklist[0];
            for (int i = 1; i < THREAD_MAX; i++) {
                struct task *tsk = &tsklist[i];
                merge(tskm->tsk_low, tsk->tsk_low - 1, tsk->tsk_high);
            } 


            auto stop3 = high_resolution_clock::now(); 
            auto duration3 = duration_cast<microseconds>(stop3 - start3);
            block3_avg_time+= duration3.count(); 
            //block_3_end 

            // pass: first parent generation

            //block4_start
            auto start4 = high_resolution_clock::now();

            auto sub_block4_start1 = high_resolution_clock::now();

            // subblock_1_start

			auto sub_start1 = high_resolution_clock::now(); 

            int linkCount = 0;
            int nodeLabel = 0;

            for(int i=0; i< totalNodes; i++){
                bfsOrder[ depthInfoVector[i].index ] = nodeLabel;
                nodeLabel++;
                parent[i] = 0;
                childrenCount[i] = -1;
                nChild[i] = 0;
                alloc[i] = 0;
            }


            for(int i=0; i< totalNodes; i++){

                if( depth[i+1] == depth[i] + 1){
                    parent[ bfsOrder[i + 1] ] = bfsOrder[i];

                    linkInfo info;
                    info.linkIndex = bfsOrder[i+1];
                    info.linkValue = bfsOrder[i];
                    linkInfoVector[linkCount] = info;
                    linkCount++;
                }
            }

            auto sub_stop1 = high_resolution_clock::now(); 
            auto sub_duration1 = duration_cast<microseconds>(sub_stop1 - sub_start1);
			sub_block4_1_avg_time+= sub_duration1.count();
			// subblock_1_end

            // pass: propagate parent link 

            // subblock_2_start

			auto sub_start2 = high_resolution_clock::now();

             int childCount;


             //begin: threading

           pthread_t linkThreads[THREAD_MAX]; 

           int chunkLen = linkCount / THREAD_MAX;

            int startChunk = 0;

            for (int i = 0; i < THREAD_MAX; i++, startChunk += chunkLen) {
                tsk = &tsklist[i];
                tsk->tsk_no = i;

                tsk->tsk_low = startChunk != 0 ? startChunk - 1 : startChunk ;
                tsk->tsk_high = startChunk + chunkLen - 1;

                if (i == (THREAD_MAX - 1))
                    tsk->tsk_high = linkCount - 1; 

                //cout<<"tsk: low, high: "<< tsk->tsk_low<<" "<< tsk->tsk_high<<endl;
 
            }

           pthread_attr_t linkAttr[THREAD_MAX];
            // creating 4 threads
            for (int i = 0; i < THREAD_MAX; i++) {
                tsk = &tsklist[i];
                pthread_attr_init(&linkAttr[i]);
                pthread_create(&linkThreads[i], &linkAttr[i], threadedLinkPropagate, tsk);
            }

            // joining all 4 threads
            for (int i = 0; i < THREAD_MAX; i++)
                pthread_join(linkThreads[i], NULL);

             //end: threading         

            auto sub_stop2 = high_resolution_clock::now(); 
            auto sub_duration2 = duration_cast<microseconds>(sub_stop2 - sub_start2);
			sub_block4_2_avg_time+= sub_duration2.count();

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

             // subblock_2_end


             //subblock_3_start
            finalOutputArraySize = totalNodes;

			auto sub_start3 = high_resolution_clock::now(); 
             // pass: Scatter child count
             for(int i=0; i< totalNodes; i++){

                if(parent[i] != parent[i+1]){
                    nChild[parent[i]] = childrenCount[i];
                    finalOutputArraySize+= childrenCount[i];	
                }
             }
             // pass: Allocate

			 resultArr= new int[finalOutputArraySize];

             for(int i=1; i<totalNodes; i++){
                alloc[i] = alloc[i-1] + nChild[i-1] + 1;
             }

            auto sub_stop3 = high_resolution_clock::now(); 
            auto sub_duration3 = duration_cast<microseconds>(sub_stop3 - sub_start3);
			sub_block4_3_avg_time+= sub_duration3.count();

             // subblock_3_end

             //last pass: generate result
             // subblock_4_start

			auto sub_start4 = high_resolution_clock::now(); 

            int iterResult= 0;
            int iterAlloc = 0;

             for(int i=0; i< totalNodes; i++){
                resultArr[iterResult++] = nChild[i];

                for(int j=0; j< nChild[i]; j++){
                    resultArr[iterResult++] = alloc[++iterAlloc];
                }
             }

            auto sub_stop4 = high_resolution_clock::now(); 
            auto sub_duration4 = duration_cast<microseconds>(sub_stop4 - sub_start4);
			sub_block4_4_avg_time+= sub_duration4.count();
             // subblock_4_end

            auto stop4 = high_resolution_clock::now(); 
            auto duration4 = duration_cast<microseconds>(stop4 - start4);
            block4_avg_time+= duration4.count(); 
          
          // block4_end
            // Get ending timepoint 
            auto stop = high_resolution_clock::now(); 
            // Get duration. Substart timepoints to  
            // get durarion. To cast it to proper unit 
            // use duration cast method 
            auto duration = duration_cast<microseconds>(stop - start); 
            average_time+= duration.count();        


       //      if(k == 0){
			    // cout<<"resultArray: ";
			    // for(int i=0; i< finalOutputArraySize; i++){
			    //     cout<<resultArr[i]<<" ";
			    // }
			    // cout<<endl; 
       //      }
            delete[] node;
			delete[] depth;
			delete[] bfsOrder;
			delete[] nChild;
			delete[] alloc;
			delete[] resultArr;

        }

        long long int actual_avg_time = (average_time/perfIteration);
        cout<<endl;
        cout<<"-------------------------------------------------------------------------------------"<<endl;
        cout<< "Input file name: "<< argv[i]<<" Input String length: "<< len <<endl;
        cout<<"Time taken by parallel version: "<< actual_avg_time << " microseconds" << endl; 

		cout<<"\% time taken by block1  : "<< ((block1_avg_time*1.0)/average_time) * 100 << "%" << endl; 
		cout<<"\% time taken by block2  : "<< ((block2_avg_time*1.0)/average_time) * 100 << "%" << endl; 
		cout<<"\% time taken by block3  : "<< ((block3_avg_time*1.0)/average_time) * 100 << "%" << endl; 
		cout<<"\% time taken by block4  : "<< ((block4_avg_time*1.0)/average_time) * 100 << "%" << endl; 

        cout<<"--------------------------------subs of block4---------------------------------"<<endl;
		cout<<"\% time taken by sub-block1  : "<< ((sub_block4_1_avg_time*1.0)/block4_avg_time) * 100 << "%" << endl; 
		cout<<"\% time taken by sub-block2  : "<< ((sub_block4_2_avg_time*1.0)/block4_avg_time) * 100 << "%" << endl; 
		cout<<"\% time taken by sub-block3  : "<< ((sub_block4_3_avg_time*1.0)/block4_avg_time) * 100 << "%" << endl; 
		cout<<"\% time taken by sub-block4  : "<< ((sub_block4_4_avg_time*1.0)/block4_avg_time) * 100 << "%" << endl;
        cout<<"-------------------------------------------------------------------------------------";
    
	delete[] depthPrefixSum.arr;
	delete[] nodePrefixSum.arr;
     
    }
    
    return 0;
}