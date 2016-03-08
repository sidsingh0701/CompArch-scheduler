#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <fstream>
#include <iomanip>
#include <bitset>
#include <math.h>
#include <algorithm>
#include <cstdlib>
#include <bitset>

using namespace std;

enum{
	IF=0,
        ID=1,
        IS=2,
        EX=3,
        WB=4
};

int l1stat=-1,l2stat=-1;

int datamem[10000];

string DecToBin2(int number)
{
    string result = "";

    do
    {
        if ( (number & 1) == 0 )
            result += "0";
        else
            result += "1";

        number >>= 1;
    } while ( number );

    reverse(result.begin(), result.end());
    return result;
}

// Splitting Address in Tag, Index and Block Offset Bits

void splitaddress(string mainbits,string& tagbits,string& setbits,string& blockbits,int tag,int sets,int blksize){
	blockbits = mainbits.substr((tag+sets),blksize);
	setbits = mainbits.substr(tag,sets);
	tagbits = mainbits.substr(0,tag);
}



class CACHE
{
private:
	int l1_read;
	int l1_read_miss;
	int l1_write;
	int l1_write_miss;
	int l1_missrate;
	int l1_numwb;
	int l1_memtraffic;
	double l1_average;
	CACHE* l1_nextlevel;
	string *l1_tagbits;
	//string *dirtybits;
	int swaps;
	int **l1_blockcount;
	int **l1_validbit;
	int **l1_dbit;
	int l2_read;
	int l2_read_miss;
	int l2_write;
	int l2_write_miss;
	int l2_missrate;
	int l2_numwb;
	int l2_memtraffic;
	double l2_average;
	CACHE* l2_nextlevel;
	string *l2_tagbits;
	int **l2_blockcount;
	int **l2_validbit;
	int **l2_dbit;
public:
	CACHE (int tag, int set_row,int sets, int blksize, int assoc,int l2_tag, int l2_set_row,int l2_sets, int l2_assoc,int vc_size,int blocksize);
	void l2_readFromAddress(string mainbits,int tag,int sets,int blksize,int repolicy,int assoc);
	void l1_readFromAddressnew(long long mainbits,int l1_tag,int l1_sets,int origblocksize, int blksize, int repolicy,int l1_assoc,int l2_size,int l2_assoc);
	void l1_checkprint(int l1_tag,int sets,int assoc,int totalsize, int blocksize);
	void l1_readFromAddress(int mainbits,int l1_tag,int l1_sets,int origblocksize, int blksize, int repolicy,int l1_assoc,int l2_size,int l2_assoc);
	void l2_checkprint(int l2_tag,int sets,int assoc,int totalsize, int blocksize);
};

CACHE::CACHE (int tag, int set_row,int sets, int blksize, int assoc,int l2_tag, int l2_set_row,int l2_sets, int l2_assoc,int vc_size,int blocksize){
	int i,j;
	l1_read = 0;
	l1_read_miss = 0;
	l1_average = 0.0;
	l1_write = 0;
	l1_write_miss = 0;
	l1_missrate = 0;
	l1_memtraffic = 0;
	l1_numwb = 0;
	swaps = 0;
	l1_nextlevel = NULL;
	l1_tagbits = new string [set_row];
	string dummy,temp;
	for(i=0;i<tag;i++)
		dummy.push_back('0');
	for(i=0;i<set_row;i++){
		temp = "";
		for(j=0;j<assoc;j++){
			temp.append(dummy);
		}
		l1_tagbits[i].append(temp);
	}
	l1_validbit = new int* [set_row];
	for(i=0;i<set_row;i++){
		l1_validbit[i] = new int [assoc]; 
	}	
	for(i=0;i<set_row;i++){
		for(j=0;j<assoc;j++){
			l1_validbit[i][j] = 0;
		}
	}
	l1_blockcount = new int* [set_row];
	for(i=0;i<set_row;i++){
		l1_blockcount[i] = new int [assoc]; 
	}
	for(i=0;i<set_row;i++){
		for(j=0;j<assoc;j++){
			l1_blockcount[i][j] = (assoc-j-1);
		}
	}	
	l1_dbit = new int* [set_row];
	for(i=0;i<set_row;i++){
		l1_dbit[i] = new int [assoc];
	}	
	for(i=0;i<set_row;i++){
		for(j=0;j<assoc;j++){
			l1_dbit[i][j] = 0;
		}
	}
	l2_read = 0;
	l2_read_miss = 0;
	l2_average = 0.0;
	l2_write = 0;
	l2_write_miss = 0;
	l2_missrate = 0;
	l2_memtraffic = 0;
	l2_numwb = 0;
	l2_nextlevel = NULL;
	string dummy2;
	l2_tagbits = new string [l2_set_row];
	//string dummy,temp;
	for(i=0;i<l2_tag;i++)
		dummy.push_back('0');
	for(i=0;i<l2_set_row;i++){
		temp = "";
		for(j=0;j<l2_assoc;j++){
			temp.append(dummy);
		}
		l2_tagbits[i].append(temp);
	}
	l2_validbit = new int* [l2_set_row];
	for(i=0;i<l2_set_row;i++){
		l2_validbit[i] = new int [l2_assoc]; 
	}	
	for(i=0;i<l2_set_row;i++){
		for(j=0;j<l2_assoc;j++){
			l2_validbit[i][j] = 0;
		}
	}
	l2_blockcount = new int* [l2_set_row];
	for(i=0;i<l2_set_row;i++){
		l2_blockcount[i] = new int [l2_assoc]; 
	}
	for(i=0;i<l2_set_row;i++){
		for(j=0;j<l2_assoc;j++){
			l2_blockcount[i][j] = (l2_assoc-j-1);
		}
	}	
	l2_dbit = new int* [l2_set_row];
	for(i=0;i<l2_set_row;i++){
		l2_dbit[i] = new int [l2_assoc];
	}	
	for(i=0;i<l2_set_row;i++){
		for(j=0;j<l2_assoc;j++){
			l2_dbit[i][j] = 0;
		}
	}
	
}

void CACHE::l1_checkprint(int l1_tag,int sets,int assoc,int totalsize,int blocksize){
int i;
double dub;
dub = totalsize/(blocksize*assoc);
	int k= dub;
 int index =0;
	int j,l,m;
 string dumb1;
 index =0;
l = 0;
 cout << "L1 CACHE CONTENTS" << endl;
cout << "a. number of accesses :" << l1_read << endl;
cout << "b. number of misses :" << l1_read_miss << endl;
 for(j=0;j<k;j++){
	cout << "set " << j << "	:";
	for(m=0;m<assoc;m++){		
			index = m*l1_tag;			
			dumb1 = l1_tagbits[j].substr(index,l1_tag);
			int check1 = strtol(dumb1.c_str(),0,2);
			printf("%X      ",check1);				
 	}
  cout << endl;
 }
}

void CACHE::l2_checkprint(int l2_tag,int sets,int assoc,int totalsize,int blocksize){
int i;
double dub;
dub = totalsize/(blocksize*assoc);
int k= dub;
 int index =0;
	int j,l,m;
 string dumb1;
 index =0;
l = 0;
 cout << "L2 CACHE CONTENTS" << endl;
cout << "a. number of accesses :" << l2_read << endl;
cout << "b. number of misses :" << l2_read_miss << endl;
 for(j=0;j<k;j++){
	cout << "set " << j << "	:";
	for(m=0;m<assoc;m++){		
			index = m*l2_tag;			
			dumb1 = l2_tagbits[j].substr(index,l2_tag);
			int check1 = strtol(dumb1.c_str(),0,2);
			printf("%X    ",check1);				
 	}
  cout << endl;
 }
}

void CACHE::l1_readFromAddress(int mainbits,int l1_tag,int l1_sets, int origblk,int blksize, int repolicy,int l1_assoc,int l2_size,int l2_assoc){
	l1_read ++;
	string realmem = DecToBin2(mainbits);
	string parse = "0";
	//cout << "hello" << endl;
	realmem.insert(0,parse);
	//cout << realmem << endl;
        int len1 = realmem.length();
	//cout << len1 << endl;	
	int lamba = 32-len1;
	while(lamba){
		realmem.insert(0,parse);
		lamba--;
	}	
	string setbits,tgbits,blockbits;
	splitaddress(realmem,tgbits,setbits,blockbits,l1_tag,l1_sets,blksize);
	int row_num;
	//cout << setbits << endl;
	row_num = strtoull(setbits.c_str(),NULL,2);
	int address_len = 32;	
	double set_bit = l2_size/(origblk*l2_assoc);
	int set_row = set_bit;
	double sets = log(set_bit)/log(2);
	int setnum = sets; //Set Bits
	//cout << "BOND IS HERE " << set_bit << endl;
	//cout << "BOND IS HERE " << sets << endl;
	//cout << "BOND IS HERE " << setnum << endl;
	int tag = address_len - setnum - blksize; // MAIN processing starts from Next line have to iterate over it
	int tagnum = tag; //Tag Size
	//cout << l1_assoc << "  **" << l1_tag << " ** " << l1_sets << endl;
	if(repolicy == 0){
		if(l1_assoc == 1){
			if((tgbits.compare(l1_tagbits[row_num]) == 0) && (l1_validbit[row_num][(l1_assoc-1)] == 1)){
				//cout << "hit" << endl;
			}
			else if(l1_validbit[row_num][(l1_assoc-1)] == 0){
				l1_read_miss ++; 
				//cout << "do u ??" << endl;
				l1_tagbits[row_num] = tgbits;
				l1_validbit[row_num][(l1_assoc-1)] = 1;
				if(l1_dbit[row_num][(l1_assoc-1)]==1){
					l1_numwb++;
				}
				l1_dbit[row_num][(l1_assoc-1)] = 0;				
				
			}
			else if(tgbits.compare(l1_tagbits[row_num]) != 0){
				l1_read_miss++;
				//cout << "do u ??" << endl;
				l1_tagbits[row_num] = tgbits;
				l1_validbit[row_num][(l1_assoc-1)] = 1;
				if(l1_dbit[row_num][(l1_assoc-1)]==1){
					l1_numwb++;
				}
				l1_dbit[row_num][(l1_assoc-1)] = 0;			
			}
		}
		else{
			int i=0,j,index=0,hit=-1,count=0,max = -100;
			string dummy;
			int point=0;
			bool flag = false;
			for(i=0;i<l1_assoc;i++){
				dummy = l1_tagbits[row_num].substr(index,l1_tag);
				hit = dummy.compare(tgbits);
				if(hit == 0){
					break;	
				}
				count++;
				index = index+l1_tag;
			}	
			if((hit == 0) && (l1_validbit[row_num][count]==1)){
				int temp = l1_blockcount[row_num][count];	
				for(i=0;i<l1_assoc;i++){
					if(l1_blockcount[row_num][i] < temp){
						l1_blockcount[row_num][i] += 1;
					}
				}
				l1_blockcount[row_num][count] = 0;
				l1stat = 1;
			}
			else {				
				for(i=0;i<l1_assoc;i++){
					if(l1_validbit[row_num][i] == 0){
						flag = true;
						point = i;
						break;
					}
				}
				if(flag == false){	
						l1_read_miss++;							
						for(i=0;i<l1_assoc;i++){
							if(max < l1_blockcount[row_num][i]){
								max = l1_blockcount[row_num][i];
								count = i;
							}
						}
						for(i=0;i<l1_assoc;i++){
							if(i == count){
								l1_blockcount[row_num][i] = 0;
							}
							else{
								l1_blockcount[row_num][i] ++;
							}
						}
						//cout << "Zone 1" << endl;
						if(l1_dbit[row_num][count]==1){ //Replace Here for Early Matching 2 nd index (assoc-1)
							string getdirty,dirty2;
							int l2_begin = count*l1_tag;
							//cout << "007  ** "<< count << "**" << l2_begin << endl;
							//cout << l1_tag;
							//cout << l2_begin<<endl;
							getdirty = l1_tagbits[row_num].substr(l2_begin,l1_tag);
							dirty2 = getdirty + setbits;
							string rand1 = dirty2.substr(0,tagnum);
							string rand2 = dirty2.substr(tagnum,setnum);
								//cout << dirty2 << endl;
								//cout << rand1 <<endl;
								//cout << rand2 << endl;
							//l2_writeToAddress(dirty2,tagnum,setnum,blksize,0,0,l2_assoc);
							l1_numwb++;	
						}
						l1_dbit[row_num][count] = 0;
						int start = count*l1_tag;
						l1_tagbits[row_num].replace(start,l1_tag,tgbits);
						//cout << tagnum << "*" << setnum << "*" <<  blksize << "*" << endl;
						//cout << mainbits << "*" << l2_assoc << endl;
						l1stat = 0;
						l2_readFromAddress(realmem,tagnum,setnum,blksize,0,l2_assoc);
				}	
				else{
						l1_read_miss++;						
						for(i=0;i<l1_assoc;i++){
							if(max < l1_blockcount[row_num][i]){
								max = l1_blockcount[row_num][i];
								count = i;
							}
						}
						for(i=0;i<l1_assoc;i++){
							if(i == count){
								l1_blockcount[row_num][i] = 0;
							}
							else{
								l1_blockcount[row_num][i] ++;
							}
						}					
						l1_validbit[row_num][count] = 1;
						if(l1_dbit[row_num][count]==1){ //SAME HERE
							string getdirty1,dirty3;
							int l2_begin = count*l1_tag;
							getdirty1 = l1_tagbits[row_num].substr(l2_begin,l1_tag);
							dirty3 = getdirty1 + setbits;
							//cout << dirty3 << endl;
							//l2_writeToAddress(dirty3,tagnum,setnum,blksize,0,0,l2_assoc);						
							l1_numwb++;
						}
						l1_dbit[row_num][count] = 0;
						int begin = count*l1_tag;
						l1_tagbits[row_num].replace(begin,l1_tag,tgbits);
						//cout << tagnum << "*" << setnum << "*" <<  blksize << "*" << endl;
						//cout << mainbits << "*" << l2_assoc << endl;
						l1stat = 0;
						l2_readFromAddress(realmem,tagnum,setnum,blksize,0,l2_assoc);
						//cout << "INVALID ZONE" << endl;
				}
			}
		}
	}	
}


//Read Function of Class handles LRU and LFU
//HAVE TO ADD VARIABLE G from Specs
// DO GENERALIZE FOR REPOLICY ND SET UP VALID BITS REMOVE BLOCKCOUNT ND CHECK AGAIN
void CACHE::l1_readFromAddressnew(long long mainbits,int l1_tag,int l1_sets, int origblk,int blksize, int repolicy,int l1_assoc,int l2_size,int l2_assoc){
	l1_read ++;
	string realmem = DecToBin2(mainbits);
	string parse = "0";
	//cout << "hello" << endl;
	realmem.insert(0,parse);
	//cout << realmem << endl;
        int len1 = realmem.length();
	//cout << len1 << endl;	
	int lamba = 32-len1;
	while(lamba){
		realmem.insert(0,parse);
		lamba--;
	}	
	string setbits,tgbits,blockbits;
	splitaddress(realmem,tgbits,setbits,blockbits,l1_tag,l1_sets,blksize);
	int row_num;
	row_num = strtoull(setbits.c_str(),NULL,2);
	//int address_len = 32;	
	//double set_bit = l2_size/(origblk*l2_assoc);
	//int set_row = set_bit;
//	double sets = log(set_bit)/log(2);
//	int setnum = sets; //Set Bits
	//cout << "BOND IS HERE " << set_bit << endl;
	//cout << "BOND IS HERE " << sets << endl;
	//cout << "BOND IS HERE " << setnum << endl;
//	int tag = address_len - setnum - blksize; // MAIN processing starts from Next line have to iterate over it
//	int tagnum = tag; //Tag Size
	//cout << l1_assoc << "  **" << l1_tag << " ** " << l1_sets << endl;
	if(repolicy == 0){
		if(l1_assoc == 1){
			if((tgbits.compare(l1_tagbits[row_num]) == 0) && (l1_validbit[row_num][(l1_assoc-1)] == 1)){
				l1stat = 1;
			}
			else if(l1_validbit[row_num][(l1_assoc-1)] == 0){
				l1_read_miss ++; 
				//cout << "do u ??" << endl;
				l1_tagbits[row_num] = tgbits;
				l1_validbit[row_num][(l1_assoc-1)] = 1;
				if(l1_dbit[row_num][(l1_assoc-1)]==1){
					l1_numwb++;
				}
				l1_dbit[row_num][(l1_assoc-1)] = 0;
				l1stat = 0;				
				
			}
			else if(tgbits.compare(l1_tagbits[row_num]) != 0){
				l1_read_miss++;
				//cout << "do u ??" << endl;
				l1_tagbits[row_num] = tgbits;
				l1_validbit[row_num][(l1_assoc-1)] = 1;
				if(l1_dbit[row_num][(l1_assoc-1)]==1){
					l1_numwb++;
				}
				l1_dbit[row_num][(l1_assoc-1)] = 0;	
				l1stat = 0;		
			}
		}
		else{
			int i=0,j,index=0,hit=-1,count=0,max = -100;
			string dummy;
			int point=0;
			bool flag = false;
			for(i=0;i<l1_assoc;i++){
				dummy = l1_tagbits[row_num].substr(index,l1_tag);
				hit = dummy.compare(tgbits);
				if(hit == 0){
					break;	
				}
				count++;
				index = index+l1_tag;
			}	
			if((hit == 0) && (l1_validbit[row_num][count]==1)){
				int temp = l1_blockcount[row_num][count];	
				for(i=0;i<l1_assoc;i++){
					if(l1_blockcount[row_num][i] < temp){
						l1_blockcount[row_num][i] += 1;
					}
				}
				l1_blockcount[row_num][count] = 0;
				l1stat = 1;
			}
			else {				
				for(i=0;i<l1_assoc;i++){
					if(l1_validbit[row_num][i] == 0){
						flag = true;
						point = i;
						break;
					}
				}
				if(flag == false){	
						l1_read_miss++;							
						for(i=0;i<l1_assoc;i++){
							if(max < l1_blockcount[row_num][i]){
								max = l1_blockcount[row_num][i];
								count = i;
							}
						}
						for(i=0;i<l1_assoc;i++){
							if(i == count){
								l1_blockcount[row_num][i] = 0;
							}
							else{
								l1_blockcount[row_num][i] ++;
							}
						}
						//cout << "Zone 1" << endl;
						if(l1_dbit[row_num][count]==1){ //Replace Here for Early Matching 2 nd index (assoc-1)
							//string getdirty,dirty2;
							//int l2_begin = count*l1_tag;
							//cout << "007  ** "<< count << "**" << l2_begin << endl;
							//cout << l1_tag;
							//cout << l2_begin<<endl;
							//getdirty = l1_tagbits[row_num].substr(l2_begin,l1_tag);
							//dirty2 = getdirty + setbits;
							//string rand1 = dirty2.substr(0,tagnum);
							//string rand2 = dirty2.substr(tagnum,setnum);
								//cout << dirty2 << endl;
								//cout << rand1 <<endl;
								//cout << rand2 << endl;
							//l2_writeToAddress(dirty2,tagnum,setnum,blksize,0,0,l2_assoc);
							l1_numwb++;	
						}
						l1_dbit[row_num][count] = 0;
						int start = count*l1_tag;
						l1_tagbits[row_num].replace(start,l1_tag,tgbits);
						l1stat = 0;
						//cout << tagnum << "*" << setnum << "*" <<  blksize << "*" << endl;
						//cout << mainbits << "*" << l2_assoc << endl;
						//l2_readFromAddress(mainbits,tagnum,setnum,blksize,0,l2_assoc);
				}	
				else{
						l1_read_miss++;						
						for(i=0;i<l1_assoc;i++){
							if(max < l1_blockcount[row_num][i]){
								max = l1_blockcount[row_num][i];
								count = i;
							}
						}
						for(i=0;i<l1_assoc;i++){
							if(i == count){
								l1_blockcount[row_num][i] = 0;
							}
							else{
								l1_blockcount[row_num][i] ++;
							}
						}					
						l1_validbit[row_num][count] = 1;
						if(l1_dbit[row_num][count]==1){ //SAME HERE
							//string getdirty1,dirty3;
							//int l2_begin = count*l1_tag;
							//getdirty1 = l1_tagbits[row_num].substr(l2_begin,l1_tag);
							//dirty3 = getdirty1 + setbits;
							//cout << dirty3 << endl;
							//l2_writeToAddress(dirty3,tagnum,setnum,blksize,0,0,l2_assoc);						
							l1_numwb++;
						}
						l1_dbit[row_num][count] = 0;
						int begin = count*l1_tag;
						l1_tagbits[row_num].replace(begin,l1_tag,tgbits);
						l1stat = 0;
						//cout << tagnum << "*" << setnum << "*" <<  blksize << "*" << endl;
						//cout << mainbits << "*" << l2_assoc << endl;
						//l2_readFromAddress(mainbits,tagnum,setnum,blksize,0,l2_assoc);
						//cout << "INVALID ZONE" << endl;
				}
			}
		}
	}	
}


void CACHE::l2_readFromAddress(string mainbits,int tag,int sets, int blksize, int repolicy,int assoc){
	l2_read++;
	string setbits,tgbits,blockbits;
	splitaddress(mainbits,tgbits,setbits,blockbits,tag,sets,blksize);
	//cout << tgbits << endl;	
	int row_num;
	row_num = strtoull(setbits.c_str(),NULL,2);
	if(repolicy == 0){
		if(assoc == 1){
			if((tgbits.compare(l2_tagbits[row_num]) == 0) && (l2_validbit[row_num][(assoc-1)] == 1)){
				//cout << "hit" << endl;
			}
			else if(l2_validbit[row_num][(assoc-1)] == 0){
				l2_read_miss ++; 
				l2_tagbits[row_num] = tgbits;
				l2_validbit[row_num][(assoc-1)] = 1;
				if(l2_dbit[row_num][(assoc-1)]==1){
					l2_numwb++;
				}
				l2_dbit[row_num][(assoc-1)] = 0;				
				
			}
			else if(tgbits.compare(l2_tagbits[row_num]) != 0){
				l2_read_miss++;
				l2_tagbits[row_num] = tgbits;
				l2_validbit[row_num][(assoc-1)] = 1;
				if(l2_dbit[row_num][(assoc-1)]==1){
					l2_numwb++;
				}
				l2_dbit[row_num][(assoc-1)] = 0;			
			}
		}
		else{
			int i=0,j,index=0,hit=-1,count=0,max = -100;
			string dummy;
			int point=0;
			bool flag = false;
			for(i=0;i<assoc;i++){
				dummy = l2_tagbits[row_num].substr(index,tag);
				hit = dummy.compare(tgbits);
				if(hit == 0){
					break;	
				}
				count++;
				index = index+tag;
			}	
			if((hit == 0) && (l2_validbit[row_num][count]==1)){
				int temp = l2_blockcount[row_num][count];	
				for(i=0;i<assoc;i++){
					if(l2_blockcount[row_num][i] < temp){
						l2_blockcount[row_num][i] += 1;
					}
				}
				l2_blockcount[row_num][count] = 0;
				l2stat = 1;
			}
			else {				
				for(i=0;i<assoc;i++){
					if(l2_validbit[row_num][i] == 0){
						flag = true;
						point = i;
						break;
					}
				}
				if(flag == false){	
						l2_read_miss++;							
						for(i=0;i<assoc;i++){
							if(max < l2_blockcount[row_num][i]){
								max = l2_blockcount[row_num][i];
								count = i;
							}
						}
						for(i=0;i<assoc;i++){
							if(i == count){
								l2_blockcount[row_num][i] = 0;
							}
							else{
								l2_blockcount[row_num][i] ++;
							}
						}
						//cout << "Zone 1" << endl;
						if(l2_dbit[row_num][count]==1){ //Replace Here for Early Matching 2 nd index (assoc-1)
							l2_numwb++;
						}
						l2_dbit[row_num][count] = 0;
						int start = count*tag;
						l2_tagbits[row_num].replace(start,tag,tgbits);
						l2stat = 0;
				}	
				else{
						l2_read_miss++;						
						for(i=0;i<assoc;i++){
							if(max < l2_blockcount[row_num][i]){
								max = l2_blockcount[row_num][i];
								count = i;
							}
						}
						for(i=0;i<assoc;i++){
							if(i == count){
								l2_blockcount[row_num][i] = 0;
							}
							else{
								l2_blockcount[row_num][i] ++;
							}
						}					
						l2_validbit[row_num][count] = 1;
						if(l2_dbit[row_num][count]==1){ //SAME HERE
							l2_numwb++;
						}
						l2_dbit[row_num][count] = 0;
						int begin = count*tag;
						l2_tagbits[row_num].replace(begin,tag,tgbits);
						l2stat = 0;
						//cout << "INVALID ZONE" << endl;
				}
			}
		}
	}	
}

struct ROB{
    int state_info;
    int op_type;
    int timer;
    int timing[5];
    int dur[5];
    int tag;
    int src1;
    int src1_stat;
    int src1_dup;
    int src2;
    int src2_stat;
    int src2_dup;
    int dest;
    int dest_dup;
    long long address;
    struct ROB *ptr;
}*front,*rear,*temp;

int reg_file[128][2];

void rob_create(){
     front = rear = NULL;
}


void rf_create(){
	int i,j;
	for(i=0;i<128;i++){
		reg_file[i][0] = 0;
		reg_file[i][1] = 1;
	}
}

int count1 = 0,cycle = 0,count_d = 0,count_i = 0, count_e = 0,hash1=130;

int wb=0;

void enqnew(long long addr,int data,int op,int src1,int src2,int dest){
    if (rear == NULL)
    {
        rear = (struct ROB *)malloc(1*sizeof(struct ROB));
        rear->ptr = NULL;
        if(op == 0)
		rear->timer = 1;
	else if(op == 1)
		rear->timer = 2;
	rear->tag = data;
        rear->op_type = op;
   	rear->src1 = src1;
  	rear->src2 = src2;
	rear->src1_stat = 1;
	rear->src2_stat = 1;        
	rear->dest = dest;
	rear->dur[0] = 1;
	rear->timing[0] = cycle;
	rear->dur[4] = 1;
	rear->src1_dup = 0;
	rear->src2_dup = 0;
	rear->dest_dup = 0;
	rear->address = addr;
	//cout << rear->a << endl;	
	rear->state_info = IF;
	front = rear;
    }
    else
    {
        temp=(struct ROB *)malloc(1*sizeof(struct ROB));
        rear->ptr = temp;
 	if(op == 0)
		temp->timer = 1;
	else if(op == 1)
		temp->timer = 2;
	temp->tag = data;
        temp->op_type = op;
   	temp->src1 = src1;
  	temp->src2 = src2;
        temp->dest = dest;
	temp->src1_dup = 0;
	temp->src2_dup = 0;
	temp->src1_stat = 1;
	temp->src2_stat = 1;
	temp->dest_dup = 0;
	temp->timing[0] = cycle;
	temp->dur[0] = 1;
	temp->dur[4] = 1;
	rear->address = addr;
	temp->state_info = IF;
        temp->ptr = NULL;
	//cout << temp->a << endl;	
        rear = temp;
    }
    count1++;
}

void enq(int data,int op,int src1,int src2,int dest){
    if (rear == NULL)
    {
        rear = (struct ROB *)malloc(1*sizeof(struct ROB));
        rear->ptr = NULL;
        if(op == 0)
		rear->timer = 1;
	else if(op == 1)
		rear->timer = 2;
	else
		rear->timer = 5;
	rear->tag = data;
        rear->op_type = op;
   	rear->src1 = src1;
  	rear->src2 = src2;
	rear->src1_stat = 1;
	rear->src2_stat = 1;        
	rear->dest = dest;
	rear->dur[0] = 1;
	rear->timing[0] = cycle;
	rear->dur[4] = 1;
	rear->src1_dup = 0;
	rear->src2_dup = 0;
	rear->dest_dup = 0;
	rear->state_info = IF;
	front = rear;
    }
    else
    {
        temp=(struct ROB *)malloc(1*sizeof(struct ROB));
        rear->ptr = temp;
 	if(op == 0)
		temp->timer = 1;
	else if(op == 1)
		temp->timer = 2;
	else
		temp->timer = 5;
	temp->tag = data;
        temp->op_type = op;
   	temp->src1 = src1;
  	temp->src2 = src2;
        temp->dest = dest;
	temp->src1_dup = 0;
	temp->src2_dup = 0;
	temp->src1_stat = 1;
	temp->src2_stat = 1;
	temp->dest_dup = 0;
	temp->timing[0] = cycle;
	temp->dur[0] = 1;
	temp->dur[4] = 1;
	temp->state_info = IF;
        temp->ptr = NULL;
        rear = temp;
    }
    count1++;
}

void display(){
   struct ROB *front1 = (struct ROB *)malloc(1*sizeof(struct ROB));
   front1 = front;
   int i;
 
    if ((front1 == NULL) && (rear == NULL))
    {
        return;
    }
    else{
    while (front1 != rear)
    {
	cout << front1->tag << " fu{" << front1->op_type << "} src{" << front1->src1 << "," << front1->src2 << "} dst{" << front1->dest << "} IF{" << front1->timing[0] << "," << front1->dur[0] << "} ID{" << front1->timing[1] << "," << front1->dur[1] << "} IS{" << front1->timing[2] << "," << front1->dur[2] << "} EX{" << front1->timing[3] << "," << front1->dur[3] << "} WB{" << front1->timing[4] << "," << front1->dur[4] << "}" << endl;
        front1 = front1->ptr;
    }
    if (front1 == rear){
	cout << front1->tag << " fu{" << front1->op_type << "} src{" << front1->src1 << "," << front1->src2 << "} dst{" << front1->dest << "} IF{" << front1->timing[0] << "," << front1->dur[0] << "} ID{" << front1->timing[1] << "," << front1->dur[1] << "} IS{" << front1->timing[2] << "," << front1->dur[2] << "} EX{" << front1->timing[3] << "," << front1->dur[3] << "} WB{" << front1->timing[4] << "," << front1->dur[4] << "}" << endl;
    }
  }
}

void dispatch2(){
   struct ROB *front1 = (struct ROB *)malloc(1*sizeof(struct ROB));
   front1 = front;
 
    if ((front1 == NULL) && (rear == NULL))
    {
        return;
    }
    else{
    while (front1 != rear)
    {
        if(front1->state_info == IF){
		front1->state_info = ID;
		front1->timing[1] = cycle;
	}
        front1 = front1->ptr;
    }
    if (front1 == rear){
	if(front1->state_info == IF){
		front1->state_info = ID;
		front1->timing[1] = cycle;
	}
    }
   }
}



void dispatch1(int size){
   struct ROB *front1 = (struct ROB *)malloc(1*sizeof(struct ROB));
   front1 = front;
 
    if ((front1 == NULL) && (rear == NULL))
    {
        return;
    }
   else{
    	while (front1 != rear)
    	{
        	if(count_i < size){		
			if(front1->state_info == ID){
				front1->state_info = IS;
				front1->timing[2] = cycle;
				front1->dur[1] = front1->timing[2]-front1->timing[1];
				if(front1->src2 >= 0){	
					if((reg_file[front1->src2][1] != 1) && (front1->src2 >= 0)){
						front1->src2_dup = reg_file[front1->src2][0];
						front1->src2_stat = 0;
		  			}	
				}
				if(front1->src1 >= 0){
					if((reg_file[front1->src1][1] != 1) && (front1->src1 >= 0)){
						front1->src1_dup = reg_file[front1->src1][0];
						front1->src1_stat = 0;
			  		}
				}
				 if(front1->dest >= 0){
					front1->dest_dup = hash1;
					reg_file[front1->dest][0] = hash1;
					reg_file[front1->dest][1] = 0;
					hash1++;
				}
				count_i++;
				count_d--;	
			}
		}
        	front1 = front1->ptr;
    }
    if (front1 == rear){
	if(count_i < size){		
		if(front1->state_info == ID){
			front1->state_info = IS;
			front1->timing[2] = cycle;
			front1->dur[1] = front1->timing[2]-front1->timing[1];
			if(front1->src2 >= 0){	
				if((reg_file[front1->src2][1] != 1) && (front1->src2 >= 0)){
					front1->src2_dup = reg_file[front1->src2][0];
					front1->src2_stat = 0;
		  		}	
			}
			if(front1->src1 >= 0){
				if((reg_file[front1->src1][1] != 1) && (front1->src1 >= 0)){
					front1->src1_dup = reg_file[front1->src1][0];
					front1->src1_stat = 0;
		  		}
			}
			if(front1->dest >= 0){
				front1->dest_dup = hash1;
				reg_file[front1->dest][0] = hash1;
				reg_file[front1->dest][1] = 0;
				hash1++;
			}	
			count_i++;
			count_d--;	
		}
	}
    }
  }
}

void issue1(int band){
   struct ROB *front1 = (struct ROB *)malloc(1*sizeof(struct ROB));
   front1 = front;
   int latency = band,count=0;
 
    if ((front1 == NULL) && (rear == NULL))
    {
        return;
    }
    else{
    while (front1 != rear)
    {
	  if(count_e < (5*band)){
		if(count<latency){	
        		if(front1->state_info == IS){	
	  			if((front1->src1_stat == 1) && (front1->src2_stat == 1)){
					front1->state_info = EX;
					if(front1->op_type == 2){
						front1->timer = 5;
					}
					front1->timing[3] = cycle;
					front1->dur[2] = (front1->timing[3] - front1->timing[2]);
					count++;
					count_i--;
					count_e++;
					front1->timer--;
				}
			}
		}
	  }

        front1 = front1->ptr;
    }
    if (front1 == rear){
 	   if(count_e < (5*band)){		
		if(count<latency){	
        		if(front1->state_info == IS){	
	  			if((front1->src1_stat == 1) && (front1->src2_stat == 1)){
					front1->state_info = EX;
					if(front1->op_type == 2){
						front1->timer = 5;
					}
					front1->timing[3] = cycle;
					front1->dur[2] = (front1->timing[3] - front1->timing[2]);
					front1->timer--;
					count++;
					count_i--;
					count_e++;
				}
			}
		}
	}
    }
   }
}

void wakeup1(int element){
   struct ROB *front1 = (struct ROB *)malloc(1*sizeof(struct ROB));
   front1 = front;
 
    if ((front1 == NULL) && (rear == NULL))
    {
        return;
    }
    else{
    while (front1 != rear)
    {	
        if(front1->src1_dup == element){	
	  	front1->src1_stat = 1;
	 }
	  if(front1->src2_dup == element){	
	  	front1->src2_stat = 1;
	 }
        front1 = front1->ptr;
    }
    if (front1 == rear){
        if(front1->src1_dup == element){	
	  	front1->src1_stat = 1;
	 }
	  if(front1->src2_dup == element){	
	  	front1->src2_stat = 1;
	 }
    }
  }
}

void execute1(int band){
   struct ROB *front1 = (struct ROB *)malloc(1*sizeof(struct ROB));
   front1 = front;
   int latency = band,count=0;
 
    if ((front1 == NULL) && (rear == NULL))
    {
        return;
    }
    else{
    while (front1 != rear)
    {	
        if(front1->state_info == EX){	
	  	if(front1->timer == 0){
			count_e--;
			front1->state_info = WB;
			front1->timing[4] = cycle;
			front1->dur[3] = (front1->timing[4] - front1->timing[3]);
			if(front1->dest_dup == reg_file[front1->dest][0]){
				reg_file[front1->dest][1] = 1;
			}
			wakeup1(front1->dest_dup);
			wb++;
		}
		else{
			front1->timer--;
		}
	 }
        front1 = front1->ptr;
    }
    if (front1 == rear){
        if(front1->state_info == EX){	
	  	if(front1->timer == 0){
			count_e--;
			front1->state_info = WB;
			front1->timing[4] = cycle;
			front1->dur[3] = (front1->timing[4] - front1->timing[3]);
			if(front1->dest_dup == reg_file[front1->dest][0]){
				reg_file[front1->dest][1] = 1;
			}
			wakeup1(front1->dest_dup);
			wb++;
		}
		else{
			front1->timer--;
		}
	}
    }
   }
}

void wb1(){
   struct ROB *front1 = (struct ROB *)malloc(1*sizeof(struct ROB));
   front1 = front;
 
    if ((front1 == NULL) && (rear == NULL))
    {
        return;
    }
    else{
    while (front1 != rear)
    {
        if(front1->state_info == WB){
		reg_file[front1->dest][1] = 1;
		wakeup1(front1->dest_dup);
	}
        front1 = front1->ptr;
    }
    if (front1 == rear){
	if(front1->state_info == IF){
		reg_file[front1->dest][1] = 1;
		wakeup1(front1->dest_dup);
	}
    }
   }
}

void fake_retire(){
    struct ROB *front_rob = (struct ROB *)malloc(1*sizeof(struct ROB));
    front_rob = front;
    if (front_rob == NULL)
    {
        printf("\n Error: Trying to display elements from empty queue");
        return;
    }
    else
        if (front_rob->ptr != NULL)
        {
            if(front_rob->state_info == WB){
		front_rob = front_rob->ptr;
            	free(front);
            	front = front_rob;
	    }
        }
        else
        {
	    if(front_rob->state_info == WB){
            	free(front);
            	front = NULL;
            	rear = NULL;
	    }
        }
        count1--;
}



void deq(){
    struct ROB *front_rob = (struct ROB *)malloc(1*sizeof(struct ROB));
    front_rob = front;
    if (front_rob == NULL)
    {
        printf("\n Error: Trying to display elements from empty queue");
        return;
    }
    else
        if (front_rob->ptr != NULL)
        {
            front_rob = front_rob->ptr;
            free(front);
            front = front_rob;
        }
        else
        {
            free(front);
            front = NULL;
            rear = NULL;
        }
        count1--;
}

int main(int argc,char *argv[]){
 int sched_size,nway,l1_size,l1_assoc,l2_size,l2_assoc,blk;
   string s;
   string super;
   FILE * pFile;
   string l1main;
   char *ass1;
   int lamba=0,len1=0,ip;
   //int lund=0;
   //int value = strtol("0", 0, 16);
   //cout << value << endl;
   string parse = "0";
   sched_size = atoi(argv[1]);
   nway = atoi(argv[2]);
   blk = atoi(argv[3]);
   l1_size = atoi(argv[4]);
   l1_assoc = atoi(argv[5]);
   l2_size = atoi(argv[6]);
   l2_assoc = atoi(argv[7]);
   ifstream infile(argv[8]);
   string filename = argv[8];
   char pc[8],mem[8];
   string realmem;
	string x,y;
   int file_len=0,i=0;
   int flag = 0;
   int op,dest,s1,s2;
   while(infile >> x >> op >> dest >> s1 >> s2 >> y){
	file_len++;
   }
   rob_create();
   rf_create();
   pFile = fopen(argv[8],"r");
   i = 0;
   int count_f = 4,j,pctag=0;
   int choos=0;
   if(l1_size == 0){
   	 while(wb<file_len){
		execute1(nway);
		issue1(nway);
		dispatch1(sched_size);
		dispatch2();
			//if(count_d < (2*nway)){
				for(j = 0;j< nway;j++){
					if(count_d < (2*nway)){
							if(fscanf(pFile,"%s %d %d %d %d %s",pc,&op,&dest,&s1,&s2,mem) != EOF){
								enq(pctag,op,s1,s2,dest);
								pctag++;
								choos++;
								count_d++;
							}
					}
				}
			//}	
		cycle++;
  	 }
	 double ipc = (double)((double)file_len/(double)cycle);
  	 display();
   	cout << "CONFIGURATION" << endl;
	cout<< " superscalar bandwidth (N) = " << nway << endl;
	cout<<" dispatch queue size (2*N) = " << (2*nway) << endl;
	cout<<" schedule queue size (S)   = " << sched_size << endl;
	cout <<"RESULTS" << endl;
	cout<<" number of instructions = " << file_len << endl;
	cout<<" number of cycles       = " << cycle << endl;
	cout<<" IPC                    = " << setprecision(2) << fixed << ipc << endl;
   }
   else{		
		double set_bit = l1_size/(blk*l1_assoc);
		double sets = log(set_bit)/log(2);
		double blksize = log(blk)/log(2);
		double tag = 32 - sets - blksize; // MAIN processing starts from Next line have to iterate over it
		int passblksize = blksize; //Block Offset
		int setnum = sets; //Set Bits
		int tagnum = tag; //Tag Size
		int set_row = set_bit;
		double set_bit1;
		double sets1=0;
		int tag1=0; // MAIN processing starts from Next line have to iterate over it
		int setnum1=0; //Set Bits
		int tagnum1=0; //Tag Size
		int set_row1=0;
		pctag = 0;
		if(l2_size != 0){		
			set_bit1 = l2_size/(blk*l2_assoc);
			sets1 = log(set_bit1)/log(2);
			tag1 = 32 - sets1 - blksize; // MAIN processing starts from Next line have to iterate over it
			setnum1 = sets1; //Set Bits
			tagnum1 = tag1; //Tag Size
			set_row1 = set_bit1;
		}
		//cout << tagnum << " " << set_row << " " << setnum << " " << passblksize << " " << tagnum1 << endl;
		CACHE cachetrial (tagnum,set_row,setnum,passblksize,l1_assoc,tagnum1,set_row1,sets1,l2_assoc,0,blk);
		pctag = 0;
		//cout << tagnum << " " << set_row << " " << setnum << " " << passblksize << " " << l1_assoc << " " << blk <<endl; 
		if(l2_size == 0){
			while(wb < file_len){
				execute1(nway);
				struct ROB *front1 = (struct ROB *)malloc(1*sizeof(struct ROB));
   front1 = front;
   int latency = nway,count=0;
 
    if ((front1 == NULL) && (rear == NULL))
    {
        //return;
    }
    else{
    while (front1 != rear)
    {
	  if(count_e < (5*nway)){
		if(count<latency){	
        		if(front1->state_info == IS){	
	  			if((front1->src1_stat == 1) && (front1->src2_stat == 1)){
					front1->state_info = EX;
					if(front1->op_type == 2){
						l1stat = -1;
						//cout << tagnum << " " << setnum << " " << blk << " " << passblksize << endl;
						//cout << datamem[front1->tag] << "*min*" << front1->tag << endl;	
						cachetrial.l1_readFromAddressnew(datamem[front1->tag],tagnum,setnum,blk,passblksize,0,l1_assoc,l2_size,l2_assoc);
						if(l1stat == 0)
							front1->timer = 20;
						else
							front1->timer = 5;
					}
					front1->timing[3] = cycle;
					front1->dur[2] = (front1->timing[3] - front1->timing[2]);
					count++;
					count_i--;
					count_e++;
					front1->timer--;
				}
			}
		}
	  }

        front1 = front1->ptr;
    }
    if (front1 == rear){
 	   if(count_e < (5*nway)){		
		if(count<latency){	
        		if(front1->state_info == IS){	
	  			if((front1->src1_stat == 1) && (front1->src2_stat == 1)){
					front1->state_info = EX;
					if(front1->op_type == 2){
						l1stat = -1;
						cachetrial.l1_readFromAddressnew(datamem[front1->tag],tagnum,setnum,blk,passblksize,0,l1_assoc,l2_size,l2_assoc);
						//cout << datamem[front1->tag] << "*kin*" << front1->tag << endl;
						if(l1stat == 0)
							front1->timer = 20;
						else
							front1->timer = 5;
					}
					front1->timing[3] = cycle;
					front1->dur[2] = (front1->timing[3] - front1->timing[2]);
					front1->timer--;
					count++;
					count_i--;
					count_e++;
				}
			}
		}
	}
    }
   }
				dispatch1(sched_size);
				dispatch2();
					if(count_d < (2*nway)){
						for(j = 0;j< nway;j++){
							if(count_d < (2*nway)){
									if(fscanf(pFile,"%s %d %d %d %d %s",pc,&op,&dest,&s1,&s2,mem) != EOF){
											long long value = strtol(mem, 0, 16);
											//cout << value << " " << pctag << endl;
											enqnew(value,pctag,op,s1,s2,dest);
											datamem[pctag] = value;
											pctag++;
											count_d++;
									}
							}
						}
					}	
				//cout << l1stat << endl;
				cycle++;
				//cout << cycle << endl;
				//	display();cout << endl;
			}
			display();
			cachetrial.l1_checkprint(tagnum,set_row,l1_assoc,l1_size,blk);
			 double ipc = (double)((double)file_len/(double)cycle);cout << endl;
			cout << "CONFIGURATION" << endl;
			cout<< " superscalar bandwidth (N) = " << nway << endl;
			cout<<" dispatch queue size (2*N) = " << (2*nway) << endl;
			cout<<" schedule queue size (S)   = " << sched_size << endl;
			cout <<"RESULTS" << endl;
			cout<<" number of instructions = " << file_len << endl;
			cout<<" number of cycles       = " << cycle << endl;
			cout<<" IPC                    = " << setprecision(2) << fixed << ipc << endl;
		}
		else{
			while(wb < file_len){
				execute1(nway);
				struct ROB *front1 = (struct ROB *)malloc(1*sizeof(struct ROB));
   front1 = front;
   int latency = nway,count=0;
 
    if ((front1 == NULL) && (rear == NULL))
    {
        //return;
    }
    else{
    while (front1 != rear)
    {
	  if(count_e < (5*nway)){
		if(count<latency){	
        		if(front1->state_info == IS){	
	  			if((front1->src1_stat == 1) && (front1->src2_stat == 1)){
					front1->state_info = EX;
					if(front1->op_type == 2){
						l1stat = -1; l2stat = -1;
						//cout << tagnum << " " << setnum << " " << blk << " " << passblksize << endl;
						//cout << datamem[front1->tag] << "*min*" << front1->tag << endl;	
						cachetrial.l1_readFromAddress(datamem[front1->tag],tagnum,setnum,blk,passblksize,0,l1_assoc,l2_size,l2_assoc);
						if(l1stat == 1)
							front1->timer = 5;
						else{
							if(l2stat == 1)
								front1->timer = 10;
							else
								front1->timer = 20;
						}
					}
					front1->timing[3] = cycle;
					front1->dur[2] = (front1->timing[3] - front1->timing[2]);
					count++;
					count_i--;
					count_e++;
					front1->timer--;
				}
			}
		}
	  }

        front1 = front1->ptr;
    }
    if (front1 == rear){
 	   if(count_e < (5*nway)){		
		if(count<latency){	
        		if(front1->state_info == IS){	
	  			if((front1->src1_stat == 1) && (front1->src2_stat == 1)){
					front1->state_info = EX;
					if(front1->op_type == 2){
						l1stat = -1; l2stat = -1;
						//cout << tagnum << " " << setnum << " " << blk << " " << passblksize << endl;
						//cout << datamem[front1->tag] << "*min*" << front1->tag << endl;	
						cachetrial.l1_readFromAddress(datamem[front1->tag],tagnum,setnum,blk,passblksize,0,l1_assoc,l2_size,l2_assoc);
						if(l1stat == 1)
							front1->timer = 5;
						else{
							if(l2stat == 1)
								front1->timer = 10;
							else
								front1->timer = 20;
						}
					}
					front1->timing[3] = cycle;
					front1->dur[2] = (front1->timing[3] - front1->timing[2]);
					front1->timer--;
					count++;
					count_i--;
					count_e++;
				}
			}
		}
	}
    }
   }
				dispatch1(sched_size);
				dispatch2();
					if(count_d < (2*nway)){
						for(j = 0;j< nway;j++){
							if(count_d < (2*nway)){
									if(fscanf(pFile,"%s %d %d %d %d %s",pc,&op,&dest,&s1,&s2,mem) != EOF){
											long long value = strtol(mem, 0, 16);
											//cout << value << " " << pctag << endl;
											enqnew(value,pctag,op,s1,s2,dest);
											datamem[pctag] = value;
											pctag++;
											count_d++;
									}
							}
						}
					}	
				//cout << l1stat << endl;
				cycle++;
				//cout << cycle << endl;
				//	display();cout << endl;
			}
			display();
			cachetrial.l1_checkprint(tagnum,set_row,l1_assoc,l1_size,blk);
			 double ipc = (double)((double)file_len/(double)cycle);cout << endl;
			cachetrial.l2_checkprint(tagnum1,set_row1,l2_assoc,l2_size,blk);cout << endl;
			cout << "CONFIGURATION" << endl;
			cout<< " superscalar bandwidth (N) = " << nway << endl;
			cout<<" dispatch queue size (2*N) = " << (2*nway) << endl;
			cout<<" schedule queue size (S)   = " << sched_size << endl;
			cout <<"RESULTS" << endl;
			cout<<" number of instructions = " << file_len << endl;
			cout<<" number of cycles       = " << cycle << endl;
			cout<<" IPC                    = " << setprecision(2) << fixed << ipc << endl;
		}
		
		//cout << cycle << endl;
   }
   return 0;
}

