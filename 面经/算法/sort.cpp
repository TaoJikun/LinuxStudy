#include <iostream>
#include <vector>
#include <stack>
#include <algorithm>

using namespace std;

/*
    1、利用临时数进行两数交换
*/
static void swap_tmp(vector<int> &arr, int i, int j)
{
    int tmp = arr[i];
    arr[i] = arr[j];
    arr[j] = tmp;
}

/*
    2、利用加减运算进行两数交换
*/
static void swap_cal(vector<int> &arr, int i, int j)
{
    if(i==j) return; //若i==j，则此数将变为0
    arr[i] = arr[i] + arr[j]; //a = a + b
    arr[j] = arr[i] - arr[j]; //b = a - b
    arr[i] = arr[i] - arr[j]; //a = a - b
}

/*
    3、利用异或运算进行两数交换(a^a=0)
*/
static void swap_xor(vector<int> &arr, int i, int j)
{
    if(i==j) return; //若i==j，则此数将变为0
    arr[i] = arr[i] ^ arr[j]; //a = a ^ b
    arr[j] = arr[i] ^ arr[j]; //b = (a ^ b) ^ b = a ^ 0 = a
    arr[i] = arr[i] ^ arr[j]; //a = (a ^ b) ^ a = a ^ 0 = b
}


/* 
    冒泡排序：不断将最大的元素沉到最后
    稳定排序
    原地排序
    最佳时间复杂度O(n)，最差时间复杂度O(n^2)，平均时间复杂度O(n^2)
    空间复杂度O(1)
*/
void BubbleSort(vector<int> &nums)
{
    //此版本最好情况的时间复杂度为O(n^2)
    if(nums.size() < 2) return;

    //i<n-1是因为第n躺结束后后n-1个数都归位了，那么第1个数其实也就归位了，就无需第n躺了
    for(int i=0;i<nums.size()-1;++i) 
    {
        for(int j=0;j<nums.size()-1-i;++j)
        {
            if(nums[j] > nums[j+1]){
                int tmp = nums[j];
                nums[j] = nums[j+1];
                nums[j+1] = tmp;
            }
        } // 这个循环走完一次一定有一个最大的元素归位
    }
}

void BubbleSort2(vector<int> &nums)
{
    //使用一个标记记录这一趟冒泡过程中是否有交换，如果没有说明已经有序
    //此版本最好情况的时间复杂度为O(n)
    if(nums.size() < 2) return;

    for(int i=0;i<nums.size()-1;++i)
    {
        bool isSwap = false;
        for(int j=0;j<nums.size()-1-i;++j)
        {
            if(nums[j] > nums[j+1]){
                isSwap = true;
                int tmp = nums[j];
                nums[j] = nums[j+1];
                nums[j+1] = tmp;
            }
        }

        if(!isSwap) break;
    }
}

void BubbleSort3(vector<int> &nums)
{
    //使用一个下标记录这一趟冒泡中最后一次发生交换的位置，其后都是有序无需扫描
    //此版本最好情况的时间复杂度为O(n)
    if(nums.size() < 2) return;

    int lastSortedIdx = nums.size()-1; //记录上一轮最后发生交换的位置
    while(lastSortedIdx > 0)
    {
        int curSortedIdx = 0;
        for(int j=0;j<lastSortedIdx;++j) //只需要扫描上一轮最后发生交换之前的位置
        {
            if(nums[j] > nums[j+1]){
                curSortedIdx = j;
                int tmp = nums[j];
                nums[j] = nums[j+1];
                nums[j+1] = tmp;
            }
        }

        lastSortedIdx = curSortedIdx;
    }
}






/*
    选择排序：前半段都是归位有序的，然后在后半段中找到最小的放到前半段尾部
    不稳定排序，例如7(1)，7(2)，2排序后变为2，7(2)，7(1)
    原地排序
    最佳时间复杂度O(n^2)，最差时间复杂度O(n^2)，平均时间复杂度O(n^2)
    空间复杂度O(1)
    注意：冒泡排序和选择排序的比较次数均为O(n^2)，但选择排序的交换次数是O(n)，而冒泡排序的平均交换次数仍然是二次的。
*/
void SelectSort(vector<int> &nums)
{
    //单元选择排序
    if(nums.size() < 2) return;

    for(int i=0;i<nums.size()-1;++i)
    {
        int minIdx = i;
        for(int j=i+1;j<nums.size();++j)
        {
            if(nums[j] < nums[minIdx]) minIdx = j;
        }

        swap(nums[i],nums[minIdx]);
    }
}

void SelectSort2(vector<int> &nums)
{
    //双元选择排序，一次遍历中同时确定最大最小值
    //该优化复杂度介于单元选择排序复杂度及其一半之间，只有系数上的区别。
    if(nums.size() < 2) return;

    int head=0,tail=nums.size()-1;
    while(head<=tail)
    {
        int minIdx = head;
        int maxIdx = head;
        for(int j=head+1;j<=tail;++j)
        {
            if(nums[j] < nums[minIdx]) minIdx = j;
            if(nums[j] > nums[maxIdx]) maxIdx = j;
        }

        if(minIdx == maxIdx) break; //说明未排序部分的值都一样，无需再排序
        swap(nums[head],nums[minIdx]);
        if(maxIdx == head) maxIdx = minIdx; //在交换head和minIdx时，有可能出现head即maxIdx的情况，此时需要修改maxIdx为minIdx
        swap(nums[tail],nums[maxIdx]);
        head++;
        tail--;
    }
}






/*
    插入排序：前半段都是归位有序的，然后选取下一个元素，找到该元素在前半段中应该的位置并插入
    稳定排序
    原地排序
    最佳时间复杂度O(n)，最差时间复杂度O(n^2)，平均时间复杂度O(n^2)
    空间复杂度O(1)
*/
void InsertSort(vector<int> &nums)
{
    if(nums.size() < 2) return;

    for(int i=1;i<nums.size();++i)
    {
        int tmp = nums[i];
        int cur = i-1;
        while(cur>=0 && tmp<nums[cur])
        {
            nums[cur+1] = nums[cur]; //将大于tmp的元素后移
            cur--;
        }
        //此时cur为-1(tmp最小)或者第一个小于tmp的数，故tmp的插入位置为cur+1
        nums[cur+1] = tmp;
    }
}

void InsertSort2(vector<int> &nums)
{
    //折半插入排序，在找某个元素的插入位置时，由于前面的元素都是有序的，故可以使用二分查找
    //找到插入位置时间降为O(nlogn)，但是由于还要移动元素，故时间还是O(n^2)
    if(nums.size() < 2) return;

    for(int i=1;i<nums.size();++i)
    {
        int tmp = nums[i];

        //使用二分法查找出tmp在前面有序序列中的位置
        int lo=0,hi=i;
        while(lo<hi){
            int mi=(lo+hi)>>1;
            //这里是要upper_bound，故应该是tmp<nums[mi]
            if(tmp < nums[mi]) hi = mi;
            else lo = mi + 1;
        }

        for(int j=i;j>lo;--j)
        {
            nums[j] = nums[j-1];
        }
        nums[lo] = tmp;
    }
}






/*
    希尔排序：给定一个增量Gap，间隔Gap取一个元素，将数组分为n组，每组进行插入排序，然后减小Gap，继续上述操作直至Gap=1
    不稳定排序
    原地排序
    时间复杂度取决于增量序列的选取
    Shell增量：n/(2^k)，最坏时间复杂度 Θ(n^2)。
    Hibbard增量：{1,3,7,15,...}，即2^k-1,k=1,2,3,...，最坏时间复杂度 Θ(n^(3/2))。
    Knuth增量：{1,4,13,40,...}，即(3^k-1)/2，k=1,2,3,...，最坏时间复杂度Θ(n^(3/2))。
    Sedgewick增量：{1,8,23,77,281...}，即4^k+3*2^(k-1)+1(最小增量1直接给出)，k=1,2,3,...，最坏时间复杂度 Θ(n^(4/3)})。
    空间复杂度O(1)
*/
void ShellSort(vector<int> &nums)
{
    if(nums.size() < 2) return;

    //初始增量使用数组长度的一半，后序增量减半(Shell增量)
    int len = (int)nums.size();

    for(int gap = len/2; gap > 0; gap/=2)
    {
        //遍历所有序列
        for(int st = 0; st<gap; ++st)
        {
            //对每个序列进行插入排序
            for(int i = st+gap; i<len; i+=gap)
            {
                int tmp = nums[i];
                int cur = i-gap;
                while(cur>=st && nums[cur]>tmp)
                {
                    nums[cur+gap] = nums[cur];
                    cur-=gap;
                }
                nums[cur+gap] = tmp;
            }
        }
    }
}






/*
    归并排序：两个有序的数组合并
    稳定排序
    非原地排序
    最佳时间复杂度O(nlogn)，最差时间复杂度O(nlogn)，平均时间复杂度O(nlogn)
    空间复杂度O(n)
*/

/*************************自顶向下非原地归并————开始****************************/
void MergeSort(vector<int> &nums, vector<int> &tmpArr, int lo, int hi)
{
    if(lo+1>=hi) return;

    int mi = (lo+hi)>>1;
    MergeSort(nums,tmpArr,lo,mi);
    MergeSort(nums,tmpArr,mi,hi);

    //备份要合并的两段
    for(int i=lo;i<hi;++i){
        tmpArr[i] = nums[i];
    }

    //合并
    int i=lo,j=mi,k=lo;
    while(i<mi&&j<hi)
    {
        if(tmpArr[i] <= tmpArr[j]) nums[k++] = tmpArr[i++];
        else nums[k++] = tmpArr[j++];
    }

    while(i<mi) nums[k++] = tmpArr[i++];
    while(j<hi) nums[k++] = tmpArr[j++];

}

void MergeSort(vector<int> &nums)
{
    if(nums.size() < 2) return;

    vector<int> tmpArr(nums.begin(),nums.end());
    MergeSort(nums,tmpArr,0,nums.size());
}
/*************************自顶向下非原地归并————结束****************************/


/*************************自底向上非原地归并————开始****************************/
void MergeSort2(vector<int> &nums)
{
    if(nums.size() < 2) return;

    int len = (int)nums.size();

    vector<int> tmpArr(nums.begin(),nums.end());

    //随着间隔的成倍增长，一一合并，二二合并，四四合并...
    for(int gap = 1;gap<len;gap*=2)
    {
        for(int lo=0;lo<len-gap;lo+=2*gap)
        {
            int hi = min(lo+2*gap, len); //防止最后一次归并越界
            int mi = lo+gap; //注意，这里不能(lo+hi)>>1，否则可能会导致后段无序

            //备份要合并的两段
            for(int i=lo;i<hi;++i){
                tmpArr[i] = nums[i];
            }

            //合并
            int i=lo,j=mi,k=lo;
            while(i<mi&&j<hi)
            {
                if(tmpArr[i] <= tmpArr[j]) nums[k++] = tmpArr[i++];
                else nums[k++] = tmpArr[j++];
            }

            while(i<mi) nums[k++] = tmpArr[i++];
            while(j<hi) nums[k++] = tmpArr[j++];
        }
    }
}
/*************************自底向上非原地归并————结束****************************/






/*
    快速排序：找到轴点分区，使得前段都不大于轴点，后段都不小于轴点
    不稳定排序
    原地排序
    最佳时间复杂度O(nlogn)，最差时间复杂度O(n^2)，平均时间复杂度O(nlogn)
    空间复杂度O(logn)
*/
void QuickSort_Head(vector<int> &nums, int lo ,int hi)
{
    //注意，这里的范围是[lo,hi]闭区间
    if(lo>=hi) return;

    //选取首元素作为轴点
    int pivot = nums[lo];
    int i=lo,j=hi;
    while(i<j)
    {
        //注意，要先走尾部的while才能使最后的i的位置即为pivot的位置，否则的话应该pivot的位置应该是i-1？
        while(i<j && nums[j]>=pivot) j--; //找到最后一个小于pivot的元素
        while(i<j && nums[i]<=pivot) i++; //找到第一个大于pivot的元素
        if(i<j) swap(nums[i],nums[j]);
    }
    swap(nums[i],nums[lo]);

    QuickSort_Head(nums,lo,i-1);
    QuickSort_Head(nums,i+1,hi);
}

void QuickSort_Random(vector<int> &nums, int lo, int hi)
{
    if(lo>=hi) return;

    //随机选取一个元素作为轴
    int randomIdx = lo + rand()%(hi-lo+1);
    int pivot = nums[randomIdx];
    swap(nums[lo],nums[randomIdx]); //将轴点交换到数组首部

    int i=lo,j=hi;
    while(i<j)
    {
        while(i<j && nums[j]>=pivot) j--; //找到最后一个小于pivot的元素
        while(i<j && nums[i]<=pivot) i++; //找到第一个大于pivot的元素
        if(i<j) swap(nums[i],nums[j]);
    }
    swap(nums[i],nums[lo]);

    QuickSort_Random(nums,lo,i-1);
    QuickSort_Random(nums,i+1,hi);
}


void QuickSort(vector<int> &nums)
{
    if(nums.size() < 2) return;

    QuickSort_Random(nums,0,nums.size()-1);
}



void QuickSort2(vector<int> &nums)
{
    //迭代版本的快速排序，使用首节点作为轴点
    if(nums.size() < 2) return;

    stack<int> sta;
    sta.push(0);
    sta.push(nums.size()-1);

    while(!sta.empty())
    {
        int hi = sta.top(); 
        sta.pop();
        int lo = sta.top();
        sta.pop();

        if(lo>=hi) continue;

        int pivot = nums[lo];
        int i=lo,j=hi;
        while(i<j)
        {
            while(i<j && nums[j]>=pivot) j--;
            while(i<j && nums[i]<=pivot) i++;

            if(i<j) swap(nums[i],nums[j]);
        }
        swap(nums[lo],nums[i]);
        sta.push(lo);
        sta.push(i-1);
        sta.push(i+1);
        sta.push(hi);
    }
}



/*
    堆排序：将数组建立为一个大顶堆，反复取出堆顶逆序排列(最先取出的放在最后)，并对剩余元素重建大顶堆，即可将原数组从小到大排列完成排序
    不稳定排序
    原地排序
    最佳时间复杂度O(nlogn)，最差时间复杂度O(nlogn)，平均时间复杂度O(nlogn)
    空间复杂度O(1)
*/
int parent(int i) { return (i-1)/2; }
int lChild(int i) { return 2*i+1; }
int rChild(int i) { return 2*i+2; }

void HeapSort_UnCorrect(vector<int> &nums)
{
    //！！！这是个错误版本的堆排序，时间复杂度达到了O(n^2)
    if(nums.size() < 2) return;

    int tail = nums.size()-1;
    while(tail > 0)
    {
        int i=tail;
        for(;i>0;--i)
        {
            if(nums[i] > nums[parent(i)]){
                swap(nums[i],nums[parent(i)]);
            }
        }

        swap(nums[0],nums[tail]);
        tail--;
    }
}

/*************************堆排序————开始****************************/
int GetMaxInPC(vector<int> &nums, int p, int n)
{
    //获取父子三者中最大的元素
    if(rChild(p) < n){
        //左右孩子均存在
        int tmpMax = (nums[lChild(p)] >= nums[rChild(p)]) ? lChild(p) : rChild(p);

        if(nums[p] >= nums[tmpMax]) return p;
        
        return tmpMax;
    }
    else if(lChild(p) < n){
        //只有左孩子存在
        if(nums[p] >= nums[lChild(p)]) return p;
        
        return lChild(p);
    }

    //左右孩子均不存在
    return p;
}

void ShiftDown(vector<int> &nums, int p, int n)
{
    int j=-1;
    while(p != (j=GetMaxInPC(nums,p,n))){
        //父节点比左或右节点小
        swap(nums[p],nums[j]);
        p=j;
    }
}

void Heapify(vector<int> &nums)
{
    //采用从下至上的下滤的方法（即Floyd算法）建堆，时间复杂度为O(n)
    //虽然将所有元素一次作插入操作的建堆方法（从上至下的上滤）时间复杂度为O(nlogn)，但是实际效率并不好
    if(nums.size() < 2) return;
    
    int r = (int)nums.size()-1;
    int j=-1;
    for(int i=parent(r);i>=0;--i)
    {
        ShiftDown(nums,i,r+1);
        /*while(i != (j=GetMaxInPC(nums,i,r+1))){
            //父节点比左或右节点小
            swap(nums[i],nums[j]);
            i=j;
        }*/
    }
}

void HeapSort(vector<int> &nums)
{
    if(nums.size() < 2) return;

    //建立大顶堆
    Heapify(nums);

    //就地排序，每次将最大的元素去除放置数组末尾，然后对前面的部分下滤
    int tail=nums.size()-1;
    while(tail > 0)
    {
        swap(nums[0], nums[tail]);
        tail--;
        ShiftDown(nums,0,tail);
    }
}
/*************************堆排序————结束****************************/






/*
    计数排序：记录每个元素出现的次数
    稳定排序
    稳定版本是非原地排序
    最佳时间复杂度O(m+n)，最差时间复杂度O(m+n)，平均时间复杂度O(m+n)。其中m为最大值减最小值
    不稳定版本空间复杂度O(m)，稳定版本空间复杂度O(m+n)
*/
void CountingSort_UnStable(vector<int> &nums)
{
    //不稳定版本
    if(nums.size() < 2) return;

    int min = nums[0], max = nums[0];
    for(int i=0;i<nums.size();++i)
    {
        if(nums[i] < min) min = nums[i];
        if(nums[i] > max) max = nums[i];
    }

    vector<int> counter(max-min+1,0);
    for(int i=0;i<nums.size();++i)
    {
        counter[nums[i]-min]++;
    }
    
    int idx=0;
    for(int i=0;i<counter.size();++i)
    {
        while(counter[i]){
            nums[idx++] = i+min;
            counter[i]--;
        }
    }
}

void CountingSort_Stable(vector<int> &nums)
{
    //稳定版本
    if(nums.size() < 2) return;

    int min = nums[0], max = nums[0];
    for(int i=0;i<nums.size();++i)
    {
        if(nums[i] < min) min = nums[i];
        if(nums[i] > max) max = nums[i];
    }

    vector<int> counter(max-min+1,0);
    for(int i=0;i<nums.size();++i)
    {
        counter[nums[i]-min]++;
    }
    
    //记录每个元素应该出现的最大位置
    for(int i=1;i<counter.size();++i)
    {
        counter[i]+=counter[i-1];
    }

    vector<int> sortedNums(nums.size());
    for(int i=nums.size()-1;i>=0;--i)
    {
        int countIdx = nums[i] - min; //nums[i]对应counter中的下标
        int sortedIdx = counter[countIdx] - 1; //在排序后的数组中的下标
        sortedNums[sortedIdx] = nums[i]; //将nums[i]就位到sortNums中
        counter[countIdx]--; //已经排了一位了，那下一位的最大位置就要减一了
    }

    nums.swap(sortedNums);
}






/*
    基数排序：从最低位开始，对元素的每一数位进行排序
    稳定排序
    非原地排序
    最佳时间复杂度O(nk)，最差时间复杂度O(n^2)，平均时间复杂度O(nk)。其中k为数字的位数
    空间复杂度O(n+k)
*/
void RadixSort(vector<int> &nums)
{
    //以稳定版本的计数排序为基础
    if(nums.size() < 2) return;

    //找到绝对值最大的数
    int max = abs(nums[0]);
    for(auto i:nums){
        if(abs(i) > max) max = abs(i);
    }

    //求绝对值最大的数的位数
    int base = 10;
    int width = 0;
    while(max){
        width++;
        max /= base;
    }

    vector<int> sortedNums(nums.size());
    //对每一个数位进行排序
    for(int i=0;i<width;++i)
    {
        vector<int> counter(19,0); //为应对有负数的情况，counter的范围为[-9,9]
        for(int k=0;k<nums.size();++k){
            int bucketIdx = (nums[k]%base)/(base/10) + 9; //当前位上的数，+9处理负数
            counter[bucketIdx]++;
        }

        for(int k=1;k<counter.size();++k)
        {
            counter[k]+=counter[k-1];
        }

        for(int k=nums.size()-1;k>=0;--k)
        {
            int counterIdx = (nums[k]%base)/(base/10) + 9;
            int sortedIdx = counter[counterIdx] - 1;
            sortedNums[sortedIdx] = nums[k];
            counter[counterIdx]--;
        }

        nums.swap(sortedNums);
        base*=10;
    }
}






/*
    桶排序：将原数组划分到称为桶的多个区间中，然后对每个桶单独进行排序，之后再按桶序和桶内序输出结果
    是否为稳定排序取决于桶内排序的方法
    非原地排序
    最佳时间复杂度O(n)，最差时间复杂度O(n^2)，平均时间复杂度O(n)
    空间复杂度O(n)
*/
void BucketSort(vector<int> &nums)
{
    if(nums.size() < 2) return;

    int n = (int)nums.size();
    int bucketNum = n/3; //这里设置n/3个桶

    int min = nums[0], max = nums[0];
    for(int i=0;i<nums.size();++i)
    {
        if(nums[i] < min) min = nums[i];
        if(nums[i] > max) max = nums[i];
    }

    vector<vector<int>> bucket(bucketNum);
    double gap = (max-min)*1.0/(bucketNum-1); //计算两个桶之间的间隔
    //将nums中的元素按照桶可容纳的范围装入对应的桶中
    for(int i=0;i<nums.size();++i)
    {
        int idx = (int)(nums[i]-min)/gap;
        bucket[idx].push_back(nums[i]);
    }

    //桶内元素排序，采取任意排序方式，这里作为示例直接使用库函数
    for(int i=0;i<bucket.size();++i)
    {
        sort(bucket[i].begin(),bucket[i].end());
    }

    int index=0;
    for(auto i:bucket)
    {
        for(auto j:i){
            nums[index++] = j;
        }
    }

}



int main()
{
    vector<int> nums = {808,84,6,684,0,84,-18,90974,-907,684};

    QuickSort2(nums);

    for(auto i:nums)
    {
        cout<<i<<"  ";
    }
    cout<<endl;
}