#include <stdio.h>//基于不同策略的英文单词的词频统计和检索系统（图形化）
#include<iostream>//引入C++输入输出库
#include <graphics.h>//引入EasyX图形库头文件
#include <string>//字符串处理库头文件
#include <math.h>//引入数学处理库头文件
#include <windows.h>//用于高精度时间计算 
#include <tchar.h>//解决字符串冲突
#include <stdlib.h>；//防止生成exe闪退

using namespace std;

#define OK 1
#define ERROR 0
#define WordMaxLength 30
#define WordMaxNum 5000
#define LIST_INIT_SIZE 100
#define LISTINCREMENT 10
typedef struct {//单词 
	char* field;//字段数组指针,存储单词对应字母的ASCII码
	int num;//字频	次数
}Word;

typedef struct {//顺序表 
	Word* elem;
	int length;//当前长度 
	int listsize;//最大长度 
}SqList;

typedef struct LNode {//链表 
	Word data;
	struct LNode* next;
}LNode, * LinkList;

typedef struct BiTNode {//二叉树 
	Word data;
	struct BiTNode* lchild, * rchild;
}BiTNode, * BiTree;

typedef struct {//基于开放地址法的哈希表 
	Word* elem;//基表 
	int number;//使用个数 此处未设space空间大小值，是因为使用算法不会产生空间不足的情况
}Table;

typedef struct WNode {//基于链地址法的哈希表专用链表 
	char* field;//字段数组指针 
	int num;//字频
	struct WNode* next;//子链 
}WNode, * WLink;

typedef struct {//基于链地址法的哈希表 
	WLink elem;//基表 
	int number;//当前数量 
}Chain;

int TableASL, ChainASL;//两个全局变量便于计算哈希表ASL值 

char* wchar_to_char(const wchar_t* wchar)//符合vs与easyX字符规则的转换
{
	char* m_char;
	int len = WideCharToMultiByte(CP_ACP, 0, wchar, wcslen(wchar), NULL, 0, NULL, NULL);
	m_char = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, wchar, wcslen(wchar), m_char, len, NULL, NULL);
	m_char[len] = '\0';
	return m_char;
}

wchar_t* char_to_wchar(const char* cchar)
{
	wchar_t* m_wchar;
	int len = MultiByteToWideChar(CP_ACP, 0, cchar, strlen(cchar), NULL, 0);
	m_wchar = new wchar_t[len + 1];
	MultiByteToWideChar(CP_ACP, 0, cchar, strlen(cchar), m_wchar, len);
	m_wchar[len] = '\0';
	return m_wchar;
}

void InitSqList(SqList& S) {//初始化顺序表 
	S.elem = (Word*)malloc(LIST_INIT_SIZE * sizeof(Word));
	if (!S.elem)exit(OVERFLOW);
	S.length = 0;
	S.listsize = LIST_INIT_SIZE;
}

void InitLinkList(LinkList& L) { //初始化单链表
	L = (LinkList)malloc(sizeof(LNode));
	if (!L)exit(OVERFLOW);
	L->next = NULL;
}

void BiTreeInsert(BiTree& T, Word* Array, int base, int top) {//向二叉树排序树中插入元素 
	if (top >= base) {
		T = (BiTree)malloc(sizeof(BiTNode));
		int median = (base + top) / 2;
		T->data.field = (char*)malloc((strlen(Array[median].field) + 1) * sizeof(char));
		strcpy_s(T->data.field, strlen(Array[median].field) + 1, Array[median].field);
		T->data.num = Array[median].num;
		T->lchild = T->rchild = NULL;
		BiTreeInsert(T->lchild, Array, base, median - 1);//创建左支树 
		BiTreeInsert(T->rchild, Array, median + 1, top); //创建右支树 
	}
}

void InitBiTree(SqList& S, BiTree& T) {//创建二叉树排序树,创建Word数组 
	Word* Array = new Word[S.length];
	for (int i = 0; i < S.length; i++) {
		Array[i].field = S.elem[i].field;
		Array[i].num = S.elem[i].num;
	}
	BiTreeInsert(T, Array, 0, S.length - 1);
	delete []Array;
}

int Hash(int number, char* wordarray) {//哈希函数:哈希值位于0到number ,number为表长，i为存储单词的次序，wordarray传入的是对应语料的单词,wordarray[i]为对应单词的ASCII码
	int hashvalue = 0;
	
	for (int i = 0; wordarray[i]; i++) {
		hashvalue += wordarray[i];
		hashvalue %= number;
		return hashvalue;
	}
}

void TableInsert(Table& B, Word& array) {//向哈希表中插入单词 
	int hashvalue = Hash(B.number, array.field);//求哈希值 
	if (!B.elem[hashvalue].field) {//未发生冲突 
		B.elem[hashvalue].field = (char*)malloc((strlen(array.field) + 1) * sizeof(char));//分配空间 
		strcpy_s(B.elem[hashvalue].field, strlen(array.field) + 1, array.field);
		B.elem[hashvalue].num = array.num;
		TableASL++;
	}
	else {//该位置已存入，使用线性探测方法处理冲突
		int i;
		for (i = 1; B.elem[(hashvalue + i) % B.number].field != NULL; i++)TableASL++;//探测地址 
		B.elem[(hashvalue + i) % B.number].field = (char*)malloc((strlen(array.field) + 1) * sizeof(char));//分配空间 
		strcpy_s(B.elem[(hashvalue + i) % B.number].field, strlen(array.field) + 1, array.field);//拷贝 
		B.elem[(hashvalue + i) % B.number].num = array.num;
	}
}

void InitTable(SqList& S, Table& B) {//借助Word数组生成一个刚好为单词数量的哈希表，空间刚好满足 
	B.elem = (Word*)malloc(S.length * sizeof(Word));
	if (!B.elem)exit(OVERFLOW);
	B.number = S.length;//单词数量 
	for (int i = 0; i < B.number; i++)B.elem[i].field = NULL;//初始化，便于检测是否发生冲突
	for (int j = 0; j < S.length; j++)TableInsert(B, S.elem[j]);
}

void SqListInsert(SqList& S, char* wordarray, int length) {//顺序表插入元素 
	//Word* p;
	if (S.length >= S.listsize) {
		Word* newbase;
		newbase = (Word*)realloc(S.elem, (S.listsize + LISTINCREMENT) * sizeof(Word));
		if (!newbase)
			exit(OVERFLOW);
		S.elem = newbase;
		S.listsize += LISTINCREMENT;
	}
	for (int i = 0; i < S.length; i++)//搜索判断是否存在相同的单词 
		if (!strcmp(S.elem[i].field, wordarray)) {
			S.elem[i].num++;
			return;
		}
	S.elem[S.length].field = (char*)malloc((length + 1) * sizeof(char));
	strcpy_s(S.elem[S.length].field, strlen(wordarray) + 1, wordarray);
	S.elem[S.length].num = 1;
	S.length++;
}

void BiTNodeDisp(BiTree& T) {//输出二叉排序树节点，按照中序是顺序输出的
	if (T) {
		BiTNodeDisp(T->lchild);//左支树 
		FILE* Outfile;
		fopen_s(&Outfile, "Outfile4.txt", "a");//采用追加的方式 
		fprintf(Outfile, "%s  %d\n", T->data.field, T->data.num);
		fclose(Outfile);
		BiTNodeDisp(T->rchild);//右支树 
	}
}

int BiTNodeCount(BiTree& T) {//计算二叉排序树的节点数量，用于词频统计 
	if (T)return BiTNodeCount(T->lchild) + BiTNodeCount(T->rchild) + 1;
	else return 0;
}

void DispFileSuccess(int tag) {//输出成功
	HWND hwnd;
	wchar_t s[200];
	hwnd = GetHWnd(); // 获取当前窗口句柄
	_stprintf_s(s, _T("词频统计成功！已导出至Outfile%d.txt文件中！\n"), tag);
	MessageBox(hwnd, s, _T("词频检索系统结果"), MB_ICONINFORMATION);
}

void DispFileBiTree(BiTree& T) {//输出排序树 
	FILE* Outfile;
	fopen_s(&Outfile, "Outfile4.txt", "w");
	fprintf(Outfile, "%d\n", BiTNodeCount(T));//写文件头 
	fclose(Outfile);
	BiTNodeDisp(T);//输出写文件体 
	DispFileSuccess(4);
}

int BiTNodeFind(BiTree& T, char* wordarray) {//二叉排序树对于结点的查找
	if (T) {
		int result = strcmp(T->data.field, wordarray);
		if (result == 0)return T->data.num;
		else if (result > 0)return BiTNodeFind(T->lchild, wordarray);//T.data->field<wordarray进左支树 
		else return BiTNodeFind(T->rchild, wordarray);
	}
	else return 0;
}

void FindSuccess(int num, double time, double asl) {
	HWND hwnd = GetHWnd();// 获取当前窗口句柄
	wchar_t s[200];
	_stprintf_s(s, _T("此单词的字频为：%d\n查找该单词所花的时间：%.2lf\n平均查找长度：%.1lf\n"), num,time,asl);
	MessageBox(hwnd, s, _T("单词查找系统结果"),MB_SETFOREGROUND);
}

void FindFail() {
	HWND hwnd;
	hwnd = GetHWnd(); // 获取当前窗口句柄
	MessageBox(hwnd, _T("查找失败,文段中没有此单词"), _T("单词查找系统结果"), MB_SETFOREGROUND| MB_ICONHAND);
}

void FindWordBiTree(BiTree& T,char *wordarray) {//二叉排序树查找 
	_LARGE_INTEGER time_start, time_end;//开始时间与结束时间 
	double dqFreq; LARGE_INTEGER f;//计时器频率
	QueryPerformanceFrequency(&f);
	dqFreq = (double)f.QuadPart;
	QueryPerformanceCounter(&time_start);//计时开始
	int num;
	num = BiTNodeFind(T, wordarray);
	QueryPerformanceCounter(&time_end);//计时结束
	if (num) {
		FindSuccess(num, 1000000 * (time_end.QuadPart - time_start.QuadPart) / dqFreq, ((BiTNodeCount(T) + 1) / BiTNodeCount(T)) * log2(BiTNodeCount(T) + 1) - 1);
	}
	else {
		FindFail();
	}
}

void DestroySqList(SqList& S) {//释放顺序表 
	for (int i = 0; i < S.length; i++)
	free(S.elem[i].field);
	free(S.elem);
	S.elem = NULL;
	S.length = 0;
	S.listsize = 0;
}

void DestoryChain(Chain& C) {//释放基于链地址法的哈希表 
	WLink p, q;
	for (int j = 0; j < C.number; j++) {
		if (C.elem[j].next == NULL)continue;
		p = C.elem[j].next;
		while (p) {
			q = p->next;
			free(p);
			p = q;
		}
		free(C.elem);
		C.number = 0;
	}
}

void DestroyLinkList(LinkList& L) {//释放链表 
	LinkList p = L->next, q = p->next;
	while (p) {
		q = p->next;
		free(p->data.field);
		free(p);
		p = NULL;
	}
	L = NULL;
}

void DestoryTable(Table& B) {//释放开放地址的哈希表 
	for (int i = 0; i < B.number; i++)
		free(B.elem[i].field);
	free(B.elem);
	B.elem = NULL;
	B.number = 0;
}

void DestoryBiTree(BiTree& T) {//释放二叉排序树 
	if (T) {
		DestoryBiTree(T->lchild);
		DestoryBiTree(T->rchild);//后序释放 
		free(T->data.field);
		free(T);
		T = NULL;
	}
}

void ChainInsert(Chain& C, Word& array) {//向链地址法的哈希表中插入元素 
	int hashvalue = Hash(C.number, array.field);
	WLink q = &C.elem[hashvalue], p = q->next;//创建哈希链地址表的表头节点
	while (p) {//p为q的后继节点
		q = p;
		p = p->next;//指向next域
		ChainASL++;//统计ASL值 
	}//找到最后一个节点，进而进行下序操作 
	p = (WNode*)malloc(sizeof(WNode));
	q->next = p;//前指针链接 
	p->next = NULL;//后指针置零 
	p->field = (char*)malloc((strlen(array.field) + 1) * sizeof(char));
	strcpy_s(p->field, strlen(array.field) + 1, array.field);
	p->num = array.num;
}

void InitChain(SqList& S, Chain& C) {//创建链地址法的哈希表 
	C.elem = (WNode*)malloc(S.length * sizeof(WNode));
	if (!C.elem)exit(OVERFLOW);
	C.number = S.length;
	for (int i = 0; i < C.number; i++)C.elem[i].next = NULL;//初始化 
	for (int i = 0; i < S.length; i++)ChainInsert(C, S.elem[i]);//插入元素 
}

//快速排序，平均时间复杂度O(N x LogN)
int Partition(Word*array, int low, int high)//快速排序更换位置 ,根据单词的ASCII码进行排序
{
	Word pivotkey;
	pivotkey.field = array[low].field; pivotkey.num = array[low].num;
	while (low < high)
	{
		//while (low < high && (array[high].num <= pivotkey.num)) //可根据词频多少排序
			while (low < high && strcmp(array[high].field, pivotkey.field)>0)
				--high;
				
		array[low].field = array[high].field; array[low].num = array[high].num;
		while (low < high && strcmp(array[low].field, pivotkey.field) < 0)
		//while (low < high && (array[high].num >= pivotkey.num))// 同287作用
			++low;
		array[high].field = array[low].field; array[high].num = array[low].num;
	}
	array[low].field = pivotkey.field; array[low].num = pivotkey.num;
	return low;
}

void QSort(Word* array, int low, int high)//顺序表快速排序，先根据首字母的ASCLL大小进行排序 ，再将相同的字母的进行二次排序
{
	if (low < high)
	{
		int pivotloc = Partition(array, low, high);
		QSort(array, low, pivotloc - 1);
		QSort(array, pivotloc + 1, high);
	}
}

void SimpleSort(LinkList& L) {//链表简单选择排序，将选择排序算法变现到链表中  时间复杂度O(N^2)
	LinkList p, q, min;
	Word temp;
	for (p = L->next; p; p = p->next) {//p为基准指针，q为选择最小值指针 
		for (q = p->next, min = p; q; q = q->next)
			if (strcmp(q->data.field, min->data.field) < 0)min = q;
		if (min != p) {
			temp.field = p->data.field; temp.num = p->data.num;//借助temp分别交换字段地址与字频 
			p->data.field = min->data.field; p->data.num = min->data.num;
			min->data.field = temp.field; min->data.num = temp.num;
		}
	}
}

void DispFileTable(Table& B) {//形成新表，存储结果
	Word* array = new Word[B.number];
	for (int i = 0; i < B.number; i++) {
		array[i].field = B.elem[i].field;
		array[i].num = B.elem[i].num;
	}
	QSort(array, 0, B.number - 1);//防止修改原表 
	FILE* Outfile;
	fopen_s(&Outfile, "Outfile5.txt", "w");
	fprintf(Outfile, "%d\n", B.number);
	for (int j = 0; j < B.number; j++)fprintf(Outfile, "%s  %d\n", array[j].field, array[j].num);
	fclose(Outfile);
	DispFileSuccess(5);
	delete []array;
}

void DispFileSqList(SqList S, int flag) {//输出文件到对应的文件中  
	FILE* Outfile;
	if (!flag)fopen_s(&Outfile, "Outfile1.txt", "w");
	else fopen_s(&Outfile, "Outfile3.txt", "w");
	fprintf(Outfile, "%d\n", S.length);
	for (int i = 0; i < S.length; i++)fprintf(Outfile, "%s  %d\n", S.elem[i].field, S.elem[i].num);
	fclose(Outfile);
	if (!flag)DispFileSuccess(1);
	else DispFileSuccess(3);
}

void DispFileLinkList(LinkList& L) {//输出文件到对应的文件中  
	FILE* Outfile=NULL;
	fopen_s(&Outfile, "Outfile2.txt", "w");
	LinkList q = L->next;
	int length;
	for (length = 0; q; q = q->next)length++;
	fprintf(Outfile, "%d\n", length);
	for (LinkList q = L->next; q; q = q->next)fprintf(Outfile, "%s  %d\n", q->data.field, q->data.num);
	fclose(Outfile);
	DispFileSuccess(2);
}

int HalfSearch(SqList& S, char* wordarray, int base, int top) {//折半查找功能区 
	if (top < base)return 0;
	else {
		int Standard = (base + top) / 2;//定义中位点 
		int Result = strcmp(S.elem[Standard].field, wordarray);
		if (Result == 0)return S.elem[Standard].num;
		else if (Result < 0)return HalfSearch(S, wordarray, Standard + 1, top);//如果小于0，则说明Standard小于wordarray，向后折半查找 
		else return HalfSearch(S, wordarray, base, Standard - 1);//如果大于0，则说明Standard大于wordarray，向前折半查找 
	}
}

void FindWordHalf(SqList& S,char *wordarray) {	//折半查找 
	_LARGE_INTEGER time_start, time_end;//开始时间与结束时间 
	double dqFreq; LARGE_INTEGER f;//计时器频率
	QueryPerformanceFrequency(&f);
	dqFreq = (double)f.QuadPart;
	QueryPerformanceCounter(&time_start);//计时开始
	int num;
	num = HalfSearch(S, wordarray, 0, S.length - 1);
	QueryPerformanceCounter(&time_end);//计时结束
	if (num) {
		FindSuccess(num, 1000000 * (time_end.QuadPart - time_start.QuadPart) / dqFreq, log(S.length) / log(2));
	}
	else {
		FindFail();
	}
}

int LinkListCount(LinkList& L) {//链表长度统计
	LinkList q = L;
	int n = 0;
	while (q = q->next)n++;
	return n;
}

void FindWordLinkList(LinkList& L,char *wordarray) {//链表顺序查找 
	_LARGE_INTEGER time_start, time_end;//开始时间与结束时间 
	double dqFreq; LARGE_INTEGER f;//计时器频率
	QueryPerformanceFrequency(&f);
	dqFreq = (double)f.QuadPart;
	QueryPerformanceCounter(&time_start);//计时开始
	LinkList p;
	for (p = L->next; p; p = p->next) {
		if (!strcmp(p->data.field, wordarray))
			break;
	}
	QueryPerformanceCounter(&time_end);//计时结束
	if (p) {
		FindSuccess(p->data.num, 1000000 * (time_end.QuadPart - time_start.QuadPart) / dqFreq, (double)(LinkListCount(L) + 1) / 2);
	}
	else {
		FindFail();
	}
}

void FindWordSqList(SqList& S,char *wordarray) {	//顺序表顺序查找 
	_LARGE_INTEGER time_start, time_end;//开始时间与结束时间 
	double dqFreq; LARGE_INTEGER f;//计时器频率
	QueryPerformanceFrequency(&f);
	dqFreq = (double)f.QuadPart;
	QueryPerformanceCounter(&time_start);//计时开始
	int i;
	for (i = 0; i < S.length; i++) {
		if (!strcmp(S.elem[i].field, wordarray))
			break;
	}
	QueryPerformanceCounter(&time_end);//计时结束
	if (i < S.length) {
		FindSuccess(S.elem[i].num, 1000000 * (time_end.QuadPart - time_start.QuadPart) / dqFreq, (double)(S.length + 1) / 2);
	}
	else {
		FindFail();
	}
}

void FindWordTable(Table& B,char *wordarray) {
	_LARGE_INTEGER time_start, time_end;//开始时间与结束时间 
	double dqFreq; LARGE_INTEGER f;//计时器频率
	QueryPerformanceFrequency(&f);
	dqFreq = (double)f.QuadPart;
	QueryPerformanceCounter(&time_start);//计时开始
	int hashvalue = Hash(B.number, wordarray);
	int i, num = 0;
	for (i = 0; i < B.number; i++)
		if (!strcmp(B.elem[(hashvalue + i) % B.number].field, wordarray)) {
			num = B.elem[(hashvalue + i) % B.number].num;
			break;
		}
	QueryPerformanceCounter(&time_end);//计时结束
	if (num) {
		FindSuccess(num, 1000000 * (time_end.QuadPart - time_start.QuadPart) / dqFreq, (double)TableASL / B.number);
	}
	else {
		FindFail();
	}
}

void FindWordChain(Chain& C,char *wordarray) {
	_LARGE_INTEGER time_start, time_end;//开始时间与结束时间 

	double dqFreq; LARGE_INTEGER f;//计时器频率
	
	QueryPerformanceFrequency(&f);
	
	dqFreq = (double)f.QuadPart;
	
	QueryPerformanceCounter(&time_start);//计时开始
	
	int hashvalue = Hash(C.number, wordarray);
	
	int num = 0;
	
	WLink p = C.elem[hashvalue].next;
	
	while (p) {
		if (!strcmp(p->field, wordarray))num = p->num;
		p = p->next;
	}
	QueryPerformanceCounter(&time_end);//计时结束
	if (num) {
		FindSuccess(num, 1000000 * (time_end.QuadPart - time_start.QuadPart) / dqFreq, (double)ChainASL / C.number);
	}
	else {
		FindFail();
	}
}

void LinkListInsert(LinkList& L, char* wordarray, int length) {//向单链表中插入元素,含头节点，方便进行操作 
	LinkList p = L->next, q = L;
	while (p) {
		if (!strcmp(p->data.field, wordarray)) {
			p->data.num++;
			break;
		}
		q = p;
		p = p->next;
	}
	if (!p) {
		p = (LinkList)malloc(sizeof(LNode));
		p->data.field = (char*)malloc((length + 1) * sizeof(char));
		strcpy_s(p->data.field, strlen(wordarray) + 1, wordarray);
		p->data.num = 1;
		q->next = p;
		p->next = NULL;
	}
}

void ReadFile(SqList& S, LinkList& L) {//读取文件，并分解成单词 ,进入到不同的数据存储结构中 
	FILE* Infile=NULL;
	if ((fopen_s(&Infile, "Infile.txt", "r")) !=0) {
		exit(1);
	}
	char wordarray[WordMaxLength], word;
	int length;
	while (1) {//将英文单词从文段中拆出  
		length = 0;
		while ((word = fgetc(Infile)) != EOF) {
			if (!(('a' <= word && word <= 'z') || ('A' <= word && word <= 'Z')) && (length == 0)) {
				continue;
			}
			else if (!(('a' <= word && word <= 'z') || ('A' <= word && word <= 'Z')) && (length > 0)) {
				break;
			}
			if ('A' <= word && word <= 'Z')wordarray[length++] = word + 32;
			else wordarray[length++] = word;
		}
		if (word != EOF) {
			wordarray[length] = '\0';
			SqListInsert(S, wordarray, length);//进入到顺序表中 
			LinkListInsert(L, wordarray, length);//进入到链表中
		}
		else break;
	}//在ReadFile时，采用的是定长读取的方式，原有的wordarray中含有其他垃圾字母，在生成 filed数组时是变长的,所以需要输入length变量以实现变长生成 
	QSort(S.elem, 0, S.length - 1);//顺序表排序
	SimpleSort(L);//链表排序 
	fclose(Infile);
}

void DispFileChain(Chain& C) {//基于链地址法的哈希表的输出 
	Word* array = new Word[C.number];//设置一个Word数组用于排序和输出 
	WLink p;
	for (int i = 0, j = 0; j < C.number; j++) {
		if (C.elem[j].next == NULL)continue;//如果是空链直接略过 
		p = C.elem[j].next;
		while (p) {
			array[i].field = p->field;
			array[i++].num = p->num;
			p = p->next;
		}
	}
	QSort(array, 0, C.number - 1);
	FILE* Outfile;
	fopen_s(&Outfile, "Outfile6.txt", "w");
	fprintf(Outfile, "%d\n", C.number);
	for (int i = 0; i < C.number; i++)fprintf(Outfile, "%s  %d\n", array[i].field, array[i].num);
	fclose(Outfile);
	DispFileSuccess(6);
}

void Insert(BiTree& T, Word e)//用以辅助建立新的去重排序二叉树
{
	if (!T)
	{
		//建立一个根节点指针S并开辟存储空间 
		BiTree S;
		T = new BiTNode;
		//为新结点S赋值 
		T->data = e;
		//初始化孩子左右结点为空 
		T->lchild = S->rchild = NULL;
		T = S; //S赋值给T 
	}
	else if (e.field < T->data.field)//如果待插入元素小于根节点，插入左边 
		Insert(T->lchild, e);
	else if (e.field > T->data.field)//大于，右边 
		Insert(T->rchild, e);
}

int DeleteLowRateBiTree(BiTree& T){//实现排序二叉树去重
	if (T)
	{
		int t = BiTNodeCount(T->lchild) + BiTNodeCount(T->rchild) + 1;//统计单词总数

		BiTree T1;
		T1 = new BiTNode;
		//T1->data = e;
		////初始化孩子左右结点为空 
		//T1->lchild = T1->rchild = NULL;
		//T = S; //S赋值给T 

		

		DeleteLowRateBiTree(T->lchild);

		if (10000 * T->data.num / t > 100)
		{
			string key1 = T->data.field;
			//DeleteBST(T,key1);
			Insert(T1, T->data);//Word e
		}
		else
		{
			//	cout<<"保留:"<<T->data.key<<" "<<10000*(double)T->data.Time/t<<endl;
		}
		DeleteLowRateBiTree(T->rchild);
	}
	return 0;
	}

int ListLength(LinkList L)//链表遍历统计总变量
{
	LinkList p;
	p = L->next;  //p指向第一个结点
	int i = 0;
	while (p) {//遍历单链表,统计结点数
		i++;
		p = p->next;
	}
	return i;
}

void Del2(LinkList& L, int no)//辅助链表去重
{
	int i; int j;
	if (no > ListLength(L) || no < 1)
	{
		cout << "Sorry，the position to be deleted is invalid!" << endl;
		return;
	}
	LinkList p, q;
	p = L; j = 0;
	while (p && (j < no - 1))
	{
		p = p->next;
		j++;
	}
	q = p->next;
	p->next = q->next;
	delete q;
	return;
}

void DelLowRateLinkList(LinkList& L)//尝试以链表去重
{
	//cout << "删除前表长:" << ListLength(L) << endl;

	int t = ListLength(L);
	int i = 0;
	
	LinkList p;
	p = L;

	

	while (p->next)
	{
		i++;
		p = p->next;
		if (10000 * (double)p->data.num / t< 100)
		{
			Del2(L, i);
			i--;//key	
		}
	}
	FILE* Outfile = NULL;
	fopen_s(&Outfile, "Outfile7.txt", "w");

	fprintf(Outfile, "%d\n", t);
	for (LinkList p = L->next; p; p = p->next)fprintf(Outfile, "%s  %d\n", p->data.field, p->data.num);
	fclose(Outfile);
	DispFileSuccess(7);

	/*cout << "删除后表长:" << ListLength(L) << endl;
	p = L;
	while (p->next)
	{
		p = p->next;
		if (10000 * (double)p->data.num / t)
		{
			cout << "剩余" << p->data.field << " " << p->data.field << " " << 10000 * (double)p->data.num / t << endl;
			
		}
	}*/
}

void Disp(SqList& S, LinkList& L, BiTree& T, Table& B, Chain& C, int tag) {//词频检索主控
	if (tag == 1)DispFileSqList(S, 0);
	if (tag == 2)DispFileLinkList(L);
	if (tag == 3)DispFileSqList(S, 1);
	if (tag == 4)DispFileBiTree(T);
	if (tag == 5)DispFileTable(B);
	if (tag == 6)DispFileChain(C);
}

void Find(SqList& S, LinkList& L, BiTree& T, Table& B, Chain& C, int tag) {//单词查找主控
	wchar_t wordarray[100];
	int i = InputBox(wordarray, 100, _T("请输入所要查找的单词"), _T("基于不同策略的英文单词查找系统"), NULL, 0, 0, false);
	if (i == false)return;
	if (tag == 1)FindWordSqList(S, wchar_to_char(wordarray));
	if (tag == 2)FindWordLinkList(L, wchar_to_char(wordarray));
	if (tag == 3)FindWordHalf(S, wchar_to_char(wordarray));
	if (tag == 4)FindWordBiTree(T, wchar_to_char(wordarray));
	if (tag == 5)FindWordTable(B, wchar_to_char(wordarray));
	if (tag == 6)FindWordChain(C, wchar_to_char(wordarray));
}

void DelLowRateSqList(SqList& S ) {//顺序表实现低频词过滤
	
	
	 SqList A; //建立新的顺序表存储去重后结果
	 InitSqList(A);//初始化新表
	 int x = 0;//新表元素序号

	for (int i = 0; i < S.length; i++)
	{
		double freq;//频率
		double t = S.elem[i].num;
		double q = S.length;
		freq = t / q;
		//转化成小数进行频率计算过滤
		if (freq >= 0.1)
		{
		
			A.listsize += LISTINCREMENT;//开辟新空间
			A.length++;//去重后长度
			A.elem[x].field = S.elem[i].field;
			A.elem[x].num = S.elem[i].num;//赋值给新表
			x++;//新表对应序号
		}
	}
	
	
	
	FILE* Outfile;
	fopen_s(&Outfile, "Outfile8.txt", "w");
	fprintf(Outfile, "%d\n", A.length);
	//fprintf(Outfile, "%d\n", t);
	for (int i = 0; i < A.length; i++)fprintf(Outfile, "%s  %d\n", A.elem[i].field, A.elem[i].num);
	fclose(Outfile);
	DispFileSuccess(8);
	//释放内存
		for (int i = 0; i < A.length; i++) {
			if (A.length != 0) {
				delete A.elem[i].field;
				free(A.elem);
				A.elem = NULL;
				A.length = 0;
				A.listsize = 0;
			}
		}
	
	
}

void  Del(SqList& S, LinkList& L,int tag){
	if (tag == 1)
	{
		DelLowRateSqList(S);
	}
	if (tag == 2)
	{
		DelLowRateLinkList(L);
	}
}
void Switch(int &value,int tag) {
	value = tag;
	HWND hwnd=GetHWnd(); // 获取当前窗口句柄
	wchar_t s[200];
	if (tag == 1)_stprintf_s(s, _T("当前策略已切换为顺序表顺序查找！\n"));
	if (tag == 2)_stprintf_s(s, _T("当前策略已切换为链表表顺序查找！\n"));
	if (tag == 3)_stprintf_s(s, _T("当前策略已切换为顺序表折半查找！\n"));
	if (tag == 4)_stprintf_s(s, _T("当前策略已切换为二叉排序树！\n"));
	if (tag == 5)_stprintf_s(s, _T("当前策略已切换为基于开放定址法的哈希表！\n"));
	if (tag == 6)_stprintf_s(s, _T("当前策略已切换为基于链地址法的哈希表！\n"));
	MessageBox(hwnd,s, _T("检索模式切换结果"), MB_ICONINFORMATION);
}
//IMAGE Interface;//定义界面图片路径为本文件夹
void Menu(SqList& S, LinkList& L, BiTree& T, Table& B, Chain& C) {
	
	initgraph(1600, 900);//设置窗口大小
	HWND Win = GetHWnd();//获取窗口句柄
	SetWindowText(Win, _T("基于不同策略的英文单词的词频统计和检索系统"));

	setbkcolor(RGB(255, 255, 255));//背景颜色设置为白色
	cleardevice();

	//loadimage(&Interface, L"word1.jpg");  //加载背景图片
	IMAGE img1,img2;
	loadimage(&img1, _T("word1.jpg"));
	loadimage(&img2, _T("title1.jpg"));
	putimage(440, 90, &img1);
	putimage(139, 0, &img2);
	
	//setfillcolor(YELLOW);//文本框体颜色
	//fillrectangle(39, 115, 265, 153);//文本框体大小
	settextcolor(RGB(0, 0, 0));//文本文字颜色
	TCHAR arr0[] =_T("退出系统"); //easyX编码问题,须进行转换
	TCHAR arr1[] = _T("顺序表策略");
	TCHAR arr2[] = _T("链表策略");
	TCHAR arr3[] = _T("折半策略");
	TCHAR arr4[] = _T("二叉排序树策略");
	TCHAR arr5[] = _T("开放定址策略");
	TCHAR arr6[] = _T("链地址法策略");
	TCHAR arr7[] = _T("词频查找");
	TCHAR arr8[] = _T("单词查找");
	TCHAR arr9[] = _T("低频词过滤");
	TCHAR arr[] = _T("哈希表实现");
	TCHAR ar[] = _T("选择所需策略");
	
	setbkmode(TRANSPARENT);
	TCHAR author1[] = _T("电子191 肖千龙");
	TCHAR author2[] = _T("自动化202 梁瀚严");
	
	
	outtextxy(39, 264, arr1);
	outtextxy(39, 314, arr2);
	outtextxy(39, 364, arr3);
	outtextxy(39, 414, arr4);
	outtextxy(39, 514, arr5);
	outtextxy(39, 564, arr6);
	outtextxy(1181, 160, arr7);
	outtextxy(1181, 252, arr8);
	outtextxy(1181, 352, arr9);

	settextstyle(32, 0, _T("Consolas"));
	outtextxy(39,464,arr);

	settextstyle(32, 0, _T("宋体"));
	settextcolor(RGB(255, 0, 0));
	outtextxy(39, 115, arr0);

	settextcolor(RGB(0, 0, 0));
	outtextxy(45, 700, author1);
	outtextxy(45, 750, author2);

	settextcolor(RGB(0, 0, 0));
	outtextxy(39, 214, ar);

	//putimage(0, 0, &Interface);//显示菜单界面
	int tag = 1;
	MOUSEMSG msg;//定义变量，保存鼠标消息
	FlushMouseMsgBuffer();// 清空鼠标消息缓冲区，避免无效鼠标信息带入到正式判断中
	while (true) {
		while (MouseHit())	//监听鼠标信息;当有鼠标消息的时候执行,可检测连续的鼠标信息
		{
			msg = GetMouseMsg();//获取鼠标消息
			if (WM_LBUTTONDOWN == msg.uMsg)//判断鼠标信息;鼠标左键按下
			{
				if (msg.x > 39 && msg.x < 265 && msg.y > 115 && msg.y < 153) {//退出
					HWND hwnd = GetHWnd();
					
					if (IDYES == MessageBox(hwnd, _T("是否确认退出本系统？"), _T("基于不同策略的英文单词的词频检索和单词查找系统"), MB_YESNO| MB_ICONQUESTION))
						exit(0);
				}
				if (msg.x > 39 && msg.x < 265 && msg.y > 264 && msg.y < 300)Switch(tag, 1);//顺序表
				if (msg.x > 39 && msg.x < 265 && msg.y > 314 && msg.y < 351)Switch(tag, 2);//链表
				if (msg.x > 39 && msg.x < 265 && msg.y > 364 && msg.y < 402)Switch(tag, 3);//折半查找
				if (msg.x > 39 && msg.x < 265 && msg.y > 414 && msg.y < 452)Switch(tag, 4);//二叉排序树
				if (msg.x > 39 && msg.x < 265 && msg.y > 515 && msg.y < 553)Switch(tag, 5);//开放定址法
				if (msg.x > 39 && msg.x < 265 && msg.y > 566 && msg.y < 603)Switch(tag, 6);//链地址法
				if (msg.x > 1181 && msg.x < 1543 && msg.y > 160 && msg.y < 224)Disp(S, L, T, B, C, tag);//词频检索
				if (msg.x > 1181 && msg.x < 1543 && msg.y > 252 && msg.y < 312)Find(S, L, T, B, C, tag);//单词查找
				if (msg.x > 1181 && msg.x < 1543 && msg.y > 352 && msg.y < 400)Del( S,L,  tag);//单词删除
				
			}
		}
	}
}
int main() {
	//int x;
	SqList S; InitSqList(S);//顺序表 
	LinkList L; InitLinkList(L);//链表 
	ReadFile(S, L);//读取文件 
	BiTree T; InitBiTree(S, T);//二叉树 
	Table B; InitTable(S, B);//基于开放地址法的哈希表 
	Chain C; InitChain(S, C);//基于链地址法的哈希表 
	Menu(S,L,T,B,C);//调用主菜单

	DestroySqList(S);//释放顺序表 
	DestroyLinkList(L);//释放链表 
	DestoryBiTree(T);//释放二叉树 
	DestoryTable(B);//释放基于开放地址法的哈希表
	DestoryChain(C);//释放链地址法的哈希表 

	system("pause");

	return 0;
	
}