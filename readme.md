# 基于不同策略的英文单词的词频统计和检索系统 #
## 北京林业大学2021-2022年春季学期 数据结构C 课程设计 ##
### 任务要求 ###
1. 读取一篇包括标点符号的英文文章（InFile.txt），假设文件中单词的个数最多不超过5000个。从文件中读取单词，过滤掉所有的标点。  
2. 分别利用线性表（包括基于顺序表的顺序查找、基于链表的顺序查找、折半查找）、二叉排序树和哈希表（包括基于开放地址法的哈希查找、基于链地址法的哈希查找）总计6种不同的检索策略构建单词的存储结构。  
3. 不论采取哪种检索策略，完成功能均相同。  
（1）词频统计  
当读取一个单词后，若该单词还未出现，则在适当的位置上添加该单词，将其词频计为1；若该单词已经出现过，则将其词频增加1。统计结束后，将所有单词及其频率按照词典顺序写入文本文件中。其中，不同的检索策略分别写入6个不同的文件。  
基于顺序表的顺序查找--- OutFile1.txt  
基于链表的顺序查找--- OutFile2.txt  
折半查找--- OutFile3.txt  
基于二叉排序树的查找--- OutFile4.txt  
基于开放地址法的哈希查找--- OutFile5.txt  
基于链地址法的哈希查找--- OutFile6.txt  
注：如果实现方法正确，6个文件的内容应该是一致的。  

### 开发环境 ##
命令行版  
VScode with gcc

窗口版  
Visual Studio 2019 C++  
X64 Release模式 字符集Unicode  
EasyX 20220116


### 开发日志 ##
2022.5.1 组队中并思考题目  
2022.5.5 组队成功，与队友讨论中  
2022.5.7 开始设计数据结构和可视化图像窗口（QT） 负责哈希表以外程序开发  
2022.5.14 顺序表读入实现ing  
2022.5.19 查阅前人blog 尝试debug前人文件   
https://www.cnblogs.com/haiyue-csdn/p/14014011.html#%E4%BB%A3%E7%A0%81%E5%B1%95%E7%A4%BA 
2022.5.21 放弃前人文件（可读性太差,哈希表排序指针有越界，不如重构）  
2022.5.22 放弃QT，学不会，改用简单的EasyX   
2022.5.27 命令行版本基本完成   
2022.5.27 移植到Visual Studio 2019 进行配置与GUI开发  
2022.5.31 差点挂了电机，先复习去了，本日对应开发EasyX的字符集问题
2022.6.03 端午假期进行排序算法优化，忘记存下优化前耗时，懒得复现了   
2022.6.07 老弟考完了高考，然而两天后考信号，三天后考单片机，本日优化了GUI   
2022.6.12 考完两门期末后补上了基于顺序表的低频词过滤，别的数据结构过滤算法有缘再说   
2022.6.13 答辩结束，哈希函数注释看起来还是不够清楚，在这也记录一下吧   
````cpp  
//本次哈希函数定义为
int Hash(int number, char* wordarray) {
	int hashvalue = 0;
	
	for (int i = 0; wordarray[i]; i++) {
		hashvalue += wordarray[i];
		hashvalue %= number;
		return hashvalue;
	}
}
typedef struct {//单词 
	char* field;//字段数组指针，是单词字母的ASCII码
	int num;//字频	次数
}Word;
char wordarray[WordMaxLength], //word;此为wordarray的结构，WordMaxLength为最大单词字母数量
//哈希函数:哈希值位于0到number ,number为表长，i为存储单词字母的次序，wordarray传入的是对应语料的单词ASCII码,wordarray[i]为对应单词的ASCII码之和

````
### 个人感悟
1.VS的编译文件真的太臃肿了，要不是EasyX窗口化开发我绝对不会使用它 
   
2.断点调试很重要，一直使用cout调试做编译会让开发效率下降    
  
3.C++的GUI开发可选远不如python和Java，再来一次我会选python的dict类型和pynlpir库配合进行开发，python的新前端工具pywebio也是可以尝试的，不过大创的Django还没弄明白，假期还得把它和vue补上  
  
4.不过数据结构的内容还得看C++，Java和Python固然简单易用，但他们毕竟是弱类语言，很多转化和C++还是大大不同的,并且C++的内存管理也是未来嵌入式开发的必修课   
  
5.一个多月时间写这个课设其实时间是相当充裕的，不过个人的拖延和期末考试挤占了大量的时间，导致最后的作品其实相当粗糙，假期得好好克服，毕竟深度学习模型的部署任务量可比写一个这种粗糙的GUI大多了
### 存在问题
1.哈希函数原理掌握不清；对哈希函数构造原理解仍需理解。   
2.对于内存管理不熟练，调试时存在野指针炸堆栈。顺序表的新表没有释放 
````cpp 
//转化成小数进行频率计算过滤
		if (freq >= 0.1)
		{
		
			A.listsize += LISTINCREMENT;//开辟新空间
			A.length++;//去重后长度
			A.elem[x].field = S.elem[i].field;
			A.elem[x].num = S.elem[i].num;//赋值给新表
			x++;//新表对应序号
		}
	
	
//问题代码 在于A 与 S 进行的是浅拷贝，S.elem查找词频后已被释放，导致多次delete内存报错
//释放内存
	for (int i = 0; i < A.length; i++) {
		delete(A.elem[i].field);//多次调用会炸堆栈，不知道问题所在
		free(A.elem);
		A.elem = NULL;
		A.length = 0;
		A.listsize = 0;
	}
```` 
或许换智能指针或是把A对S的拷贝改为深拷贝？有待尝试
3.答辩容易紧张，对分治算法不熟练。   
4.低频词过滤仍未完善，只完成了顺序表的低频词过滤   

