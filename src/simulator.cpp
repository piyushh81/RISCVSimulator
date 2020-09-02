// Task 3: Pipelining

//bool en2....int
//make change
#include <bits/stdc++.h>
#define lli long long int
using namespace std;

#define OFF 0
#define ON 1
#define TRUE 0
#define FALSE 1

#define OPCODE_I1 3   //for load
#define OPCODE_I2 19  // for shift, ori, andi
#define OPCODE_I3 27  // for shiftw and addiw
#define OPCODE_I4 103 //for jalr

#define OPCODE_U1 23 // for auipc
#define OPCODE_U2 55 // for lui

#define OPCODE_S1 35 //for sd, sw, sl, sh

#define OPCODE_R1 51 //for add, sub, and etc
#define OPCODE_R2 59 // for addw, subw etc

#define OPCODE_SB1 99 //for branch jump

#define OPCODE_UJ 111 //for jal

#define FETCH_STAGE 1
#define DECODE_STAGE 2
#define EXECUTE_STAGE 3
#define MEM_STAGE 4
#define WB_STAGE 5

#define NO_DATA_DEPEND 0
#define DATA_DEPEND_RA 1
#define DATA_DEPEND_RB 2
#define DATA_DEPEND_RA_RB 3
#define DATA_DEPENED_MtoM 4

struct dmCache
{
	bool validBit;
	unsigned char *data;
	unsigned int tag;
	dmCache()
	{
		validBit = 0;
	}
};

struct faCache
{
	bool validBit;
	unsigned char *data;
	unsigned int tag;
	int lru;
	faCache()
	{
		validBit = 0;
		lru = 0;
	}
};

struct saCache
{
	bool validBit;
	unsigned char *data;
	unsigned int tag;
	int lru;
	saCache()
	{
		validBit = 0;
		lru = 0;
	}
};

bool knob1, knob2, knob3, knob4, knob5;
int n, m, k;

dmCache *DMCache1; //instruction cache
dmCache *DMCache2; //data cache
faCache *FACache1;
faCache *FACache2;
saCache **SACache1;
saCache **SACache2;

unsigned char memory[1 << 24]; //Processor Memory
int regArray[32] = {0};

int hits;
int cacheType;
int memoryAccesses;
int conflictMisses;
int capacityMisses;
int coldMisses;

int isLoadInstruction = 0;
int isPrevLoadInstruction = 0;
int aluInstructions = 0;
int controlInstructions = 0;
int dataTransferInstructions = 0;

int stalls = 0;
int stalls_data_hazard = 0;
int stalls_control_hazard = 0;

int data_hazard = 0;
int control_hazard = 0;
int branch_mispredictions = 0;

int cycleCount = 0;
int total_instructions = 0;
double CPI = 0;

//Processor Memory Interface
int readWriteMemory(int MEM_READ, int MEM_WRITE, int address = 0, int data_w = 0)
{
	if (MEM_READ > 0)
	{
		if (MEM_READ == 1) //lb
			return memory[address];
		else if (MEM_READ == 2)
		{ //lh
			int data = memory[address];
			data += (int)memory[address + 1] << 8;
			return data;
		}
		else if (MEM_READ == 3)
		{ //lw
			int data = memory[address];
			data += (int)memory[address + 1] << 8;
			data += (int)memory[address + 2] << 16;
			data += (int)memory[address + 3] << 24;
			return data;
		}
	}
	if (MEM_WRITE == 1)
	{ //sb
		memory[address] = data_w;
	}
	else if (MEM_WRITE == 2)
	{ //sh
		memory[address] = data_w & ((1 << 8) - 1);
		memory[address + 1] = data_w >> 8;
	}
	else if (MEM_WRITE == 3)
	{ //sw
		int setb8 = (1 << 8) - 1;
		memory[address] = data_w & setb8;
		memory[address + 1] = (data_w & (setb8 << 8)) >> 8;
		memory[address + 2] = (data_w & (setb8 << 16)) >> 16;
		memory[address + 3] = data_w >> 24;
	}
	return 0;
}
//End of readWriteMemory

//Print stats in stats.txt file
void stats_print()
{
	total_instructions = aluInstructions + controlInstructions + dataTransferInstructions;
	stalls = stalls_control_hazard + stalls_data_hazard;
	control_hazard = controlInstructions;
	CPI = (double)cycleCount / (double)total_instructions;
	fstream fileWriting;
	fileWriting.open("stats.txt", ios::out);
	fileWriting << "----------------------------------------------------------------------" << endl;
	fileWriting << "Total Cycles               :  " << cycleCount << endl;
	fileWriting << "Total Instructions         :  " << total_instructions << endl;
	fileWriting << "CPI                        :  " << CPI << endl;
	fileWriting << "----------------------------------------------------------------------" << endl;
	fileWriting << "Data-Transfer Instructions :  " << dataTransferInstructions << endl;
	fileWriting << "ALU Instruction            :  " << aluInstructions << endl;
	fileWriting << "Control Instructions       :  " << controlInstructions << endl;
	fileWriting << "----------------------------------------------------------------------" << endl;
	fileWriting << "Data Hazards               :  " << data_hazard << endl;
	fileWriting << "Control Hazards            :  " << control_hazard << endl;
	fileWriting << "----------------------------------------------------------------------" << endl;
	fileWriting << "Stalls due Data Hazard     :  " << stalls_data_hazard << endl;
	fileWriting << "Stalls due Control Hazard  :  " << stalls_control_hazard << endl;
	fileWriting << "Total Stalls               :  " << stalls << endl;
	fileWriting << "----------------------------------------------------------------------" << endl;
	fileWriting << "Branch Mis-predictions     :  " << branch_mispredictions << endl;
	fileWriting << "----------------------------------------------------------------------" << endl;
	fileWriting << "Cache Hits                 :  " << hits << endl;
	fileWriting << "Memory Access              :  " << memoryAccesses << endl;
	fileWriting << "Conflict Misses            :  " << conflictMisses << endl;
	fileWriting << "Capacity Misses            :  " << capacityMisses << endl;
	fileWriting << "Cold Misses                :  " << coldMisses << endl;
	cout << "----------------------------------------------------------------------" << endl;
	fileWriting.close();
}
//End of stats_print()

//Prints all register file & their value
void printRegisterFile()
{
	fstream fileWriting;
	fileWriting.open("regFile.txt", ios::app);
	fileWriting << "----------------------------------------------------------------------" << endl;
	fileWriting << "CYCLE NUMBER\t\t:\t" << cycleCount << endl;
	for (int i = 0; i < 10; i++)
		fileWriting << "REGISTER x" << i << "\t\t\t:\t" << regArray[i] << endl;
	for (int i = 10; i < 32; i++)
		fileWriting << "REGISTER x" << i << "\t\t:\t" << regArray[i] << endl;
	fileWriting << "----------------------------------------------------------------------" << endl;
	fileWriting.close();
}
//Check data dependency Execute to Execute

//Prints memory that has been alloted with data or instruction!
void printMemory()
{
	fstream fileWriting;
	fileWriting.open("memory.txt", ios::out);
	fileWriting << "----------------------------------------------------------------------" << endl;
	for (int i = 0; i < 1 << 22; i++)
		if (memory[i] != '\0')
			fileWriting << i << "\t" << (int)memory[i] << endl;
	fileWriting << "----------------------------------------------------------------------" << endl;
	fileWriting.close();
}
//End of printMemory

int readWriteSACache(saCache **SACache, int n, int m, int k /*k-way*/, int MEM_READ, int MEM_WRITE, int address = 0, int data_w = 0)
{
	int x = 1 << (m + 2);
	int MOD = 1 << n;
	unsigned int temp = address >> (m + 2);
	unsigned int Index = temp % MOD;
	unsigned int Tag = temp >> n;
	unsigned int i1 = (address) % x;
	unsigned int i2 = (address + 1) % x;
	unsigned int i3 = (address + 2) % x;
	unsigned int i4 = (address + 3) % x;
	if (MEM_READ > 0)
	{
		int flag = 0;
		for (int i = 0; i < k; i++)
		{
			if (SACache[Index][i].tag == Tag && SACache[Index][i].validBit == 1)
			{
				flag = 1;
				for (int j = 0; j < k; j++)
				{
					SACache[Index][j].lru += 1;
				}
				SACache[Index][i].lru = 0;
				if (MEM_READ == 1)
				{
					return (int)SACache[Index][i].data[i1];
				}
				if (MEM_READ == 2)
				{
					int y = SACache[Index][i].data[i1];
					y += (int)(SACache[Index][i].data[i2] << 8);
					return y;
				}
				if (MEM_READ == 3)
				{
					int y = SACache[Index][i].data[i1];
					y += (int)SACache[Index][i].data[i2] << 8;
					y += (int)SACache[Index][i].data[i3] << 16;
					y += (int)SACache[Index][i].data[i4] << 24;
					return y;
				}
			}
		}
		if (flag == 0) //it's a miss (not found in cache)
		{
			int flag1 = 0; //1 if empty location found
			int e = 0;	 //empty location index
			for (int i = 0; i < k; i++)
			{
				if (SACache[Index][i].validBit == 0)
				{
					flag1 = 1;
					e = i;
					break;
				}
			}
			if (flag1 == 1)
			{
				coldMisses++;
				SACache[Index][e].tag = Tag;
				//SACache[Index][e].validBit=1;
				for (int j = 0; j < k; j++)
					SACache[Index][j].lru++;
				SACache[Index][e].lru = 0;
				for (int i = 0; i < (1 << (m + 2)); i++)
					SACache[Index][e].data[(address + i) % x] = memory[address + i];
				if (MEM_READ == 1)
					return (int)memory[address];
				if (MEM_READ == 2)
				{
					int y = memory[address];
					y += (int)memory[address + 1] << 8;
					return y;
				}
				if (MEM_READ == 3)
				{
					int y = memory[address];
					y += (int)memory[address + 1] << 8;
					y += (int)memory[address + 2] << 16;
					y += (int)memory[address + 3] << 24;
					return y;
				}
			}
			else
			{
				int flag2 = 0; //1 if there is atleast 1 empty location in entire cache
				for (int i = 0; i < (1 << n); i++)
				{
					for (int j = 0; j < k; j++)
					{
						if (SACache[i][j].validBit == 0)
							flag2 = 1;
					}
				}
				if (flag2 == 1)
					conflictMisses++;
				else
					capacityMisses++;
				int e1 = 0;
				int max = 0;
				for (int j = 0; j < k; j++)
				{
					if (SACache[Index][j].lru > max)
					{
						max = SACache[Index][j].lru;
					}
				}
				for (int j = 0; j < k; j++)
				{
					if (SACache[Index][j].lru == max)
						e1 = j;
				}
				for (int j = 0; j < k; j++)
					SACache[Index][j].lru++;
				SACache[Index][e1].lru = 0;
				SACache[Index][e1].tag = Tag;
				for (int i = 0; i < (1 << (m + 2)); i++)
					SACache[Index][e1].data[(address + i) % x] = memory[address + i];
				if (MEM_READ == 1)
					return memory[address];
				if (MEM_READ == 2)
				{
					int y = memory[address];
					y += (int)memory[address + 1] << 8;
					return y;
				}
				if (MEM_READ == 3)
				{
					int y = memory[address];
					y += (int)memory[address + 1] << 8;
					y += (int)memory[address + 2] << 16;
					y += (int)memory[address + 3] << 24;
					return y;
				}
			}
		}
	}
	else
	{
		int flag3 = 0;
		int e = 0;
		for (int i = 0; i < k; i++)
		{
			if (SACache[Index][i].tag == Tag && SACache[Index][i].validBit == 1)
			{
				flag3 = 1;
				e = i;
			}
		}
		if (flag3 == 1)
		{
			if (MEM_WRITE == 1)
			{
				SACache[Index][e].tag = Tag;
				SACache[Index][e].data[i1] = data_w;
				for (int i = 1; i < (1 << (m + 2)); i++)
					SACache[Index][e].data[(address + i) % x] = memory[address + i];
				memory[address] = data_w;
			}
			if (MEM_WRITE == 2)
			{
				SACache[Index][e].data[i1] = data_w;
				SACache[Index][e].data[i2] = data_w >> 8;
				for (int i = 2; i < (1 << (m + 2)); i++)
					SACache[Index][e].data[(address + i) % x] = memory[address + i];
				memory[address] = data_w & ((1 << 8) - 1);
				memory[address + 1] = data_w >> 8;
			}
			if (MEM_WRITE == 3)
			{
				int setb8 = (1 << 8) - 1;
				SACache[Index][e].data[i1] = data_w & setb8;
				SACache[Index][e].data[i2] = (data_w & (setb8 << 8)) >> 8;
				SACache[Index][e].data[i3] = (data_w & (setb8 << 16)) >> 16;
				SACache[Index][e].data[i4] = data_w >> 24;
				memory[address] = data_w & setb8;
				memory[address + 1] = (data_w & (setb8 << 8)) >> 8;
				memory[address + 2] = (data_w & (setb8 << 16)) >> 16;
				memory[address + 3] = data_w >> 24;
				for (int i = 4; i < (1 << (m + 2)); i++)
				{
					SACache[Index][e].data[(address + i) % x] = memory[address + i];
				}
			}
		}
		else
		{
			if (MEM_WRITE == 1)
			{ //sb
				memory[address] = data_w;
			}
			else if (MEM_WRITE == 2)
			{ //sh
				memory[address] = data_w & ((1 << 8) - 1);
				memory[address + 1] = data_w >> 8;
			}
			else if (MEM_WRITE == 3)
			{ //sw
				int setb8 = (1 << 8) - 1;
				memory[address] = data_w & setb8;
				memory[address + 1] = (data_w & (setb8 << 8)) >> 8;
				memory[address + 2] = (data_w & (setb8 << 16)) >> 16;
				memory[address + 3] = data_w >> 24;
			}
		}
		return 0;
	}
}

int readWriteFACache(faCache *FACache, int n /*num_blocks=2^n*/, int m /*block_size=2^m words*/, int MEM_READ, int MEM_WRITE, int address = 0, int data_w = 0)
{
	int x = 1 << (m + 2);
	unsigned int Tag = address >> (m + 2);
	unsigned int i1 = (address) % x;
	unsigned int i2 = (address + 1) % x;
	unsigned int i3 = (address + 2) % x;
	unsigned int i4 = (address + 3) % x;
	int flag = 0, flag1 = 0;
	if (MEM_READ > 0)
	{
		for (int i = 0; i < (1 << n); i++)
		{
			if (FACache[i].validBit == 1 && FACache[i].tag == Tag)
			{
				flag = 1;
				for (int j = 0; j < (1 << n); j++)
				{
					FACache[j].lru++;
				}
				FACache[i].lru = 0;
				if (MEM_READ == 1)
					return (int)FACache[i].data[i1];
				if (MEM_READ == 2)
				{
					int y = FACache[i].data[i1];
					y += (int)((FACache[i].data[i2]) << 8);
					return y;
				}
				if (MEM_READ == 3)
				{
					int y = FACache[i].data[i1];
					y += (int)FACache[i].data[i2] << 8;
					y += (int)FACache[i].data[i3] << 16;
					y += (int)FACache[i].data[i4] << 24;
					return y;
				}
			}
		}
		if (flag == 0) //required data not found in cache(miss)
		{
			int e = 0;
			for (int i = 0; i < (1 << n); i++)
			{
				if (FACache[i].validBit == 0)
				{
					flag1++;
					e = i;
					break;
				}
			}
			if (flag1 == 1) //cache is yet not full
			{
				FACache[e].tag = Tag;
				coldMisses++;
				for (int j = 0; j < (1 << n); j++)
					FACache[j].lru++;
				FACache[e].lru = 0;
				for (int i = 0; i < (1 << (m + 2)); i++)
					FACache[e].data[(address + i) % x] = memory[address + i];
				if (MEM_READ == 1)
					return memory[address];
				if (MEM_READ == 2)
				{
					int y = memory[address];
					y += (int)memory[address + 1] << 8;
					return y;
				}
				if (MEM_READ == 3)
				{
					int y = memory[address];
					y += (int)memory[address + 1] << 8;
					y += (int)memory[address + 2] << 16;
					y += (int)memory[address + 3] << 24;
					return y;
				}
			}
			else //cache is full, capacity miss
			{
				capacityMisses++;
				int e1 = 0;
				int max = 0;
				for (int j = 0; j < (1 << n); j++)
				{
					if (FACache[j].lru > max)
					{
						max = FACache[j].lru;
					}
				}
				for (int j = 0; j < (1 << n); j++)
				{
					if (FACache[j].lru == max)
						e1 = j;
					FACache[j].lru++;
				}
				FACache[e1].lru = 0;
				FACache[e1].tag = Tag;
				for (int i = 0; i < (1 << (m + 2)); i++)
					FACache[e1].data[(address + i) % x] = memory[address + i];
				if (MEM_READ == 1)
					return memory[address];
				if (MEM_READ == 2)
				{
					int y = memory[address];
					y += (int)memory[address + 1] << 8;
					return y;
				}
				if (MEM_READ == 3)
				{
					int y = memory[address];
					y += (int)memory[address + 1] << 8;
					y += (int)memory[address + 2] << 16;
					y += (int)memory[address + 3] << 24;
					return y;
				}
			}
		}
	}
	else
	{
		flag = 0;
		int e2 = 0; //index of required data(block)
		for (int i = 0; i < (1 << n); i++)
		{
			if (FACache[i].validBit == 1 && FACache[i].tag == Tag)
			{
				e2 = i;
				flag = 1;
				break;
			}
		}
		if (flag == 1)
		{
			if (MEM_WRITE == 1)
			{
				FACache[e2].tag = Tag;
				FACache[e2].data[i1] = data_w;
				for (int i = 1; i < (1 << (m + 2)); i++)
					FACache[e2].data[(address + i) % x] = memory[address + i];
				memory[address] = data_w;
			}
			if (MEM_WRITE == 2)
			{
				FACache[e2].data[i1] = data_w;
				FACache[e2].data[i2] = data_w >> 8;
				for (int i = 2; i < (1 << (m + 2)); i++)
					FACache[e2].data[(address + i) % x] = memory[address + i];
				memory[address] = data_w & ((1 << 8) - 1);
				memory[address + 1] = data_w >> 8;
			}
			if (MEM_WRITE == 3)
			{
				int setb8 = (1 << 8) - 1;
				FACache[e2].data[i1] = data_w & setb8;
				FACache[e2].data[i2] = (data_w & (setb8 << 8)) >> 8;
				FACache[e2].data[i3] = (data_w & (setb8 << 16)) >> 16;
				FACache[e2].data[i4] = data_w >> 24;
				memory[address] = data_w & setb8;
				memory[address + 1] = (data_w & (setb8 << 8)) >> 8;
				memory[address + 2] = (data_w & (setb8 << 16)) >> 16;
				memory[address + 3] = data_w >> 24;
				for (int i = 4; i < (1 << (m + 2)); i++)
				{
					FACache[e2].data[(address + i) % x] = memory[address + i];
				}
			}
		}
		else
		{
			if (MEM_WRITE == 1)
			{ //sb
				memory[address] = data_w;
			}
			else if (MEM_WRITE == 2)
			{ //sh
				memory[address] = data_w & ((1 << 8) - 1);
				memory[address + 1] = data_w >> 8;
			}
			else if (MEM_WRITE == 3)
			{ //sw
				int setb8 = (1 << 8) - 1;
				memory[address] = data_w & setb8;
				memory[address + 1] = (data_w & (setb8 << 8)) >> 8;
				memory[address + 2] = (data_w & (setb8 << 16)) >> 16;
				memory[address + 3] = data_w >> 24;
			}
			return 0;
		}
	}
}

int readWriteDMCache(dmCache *DMCache, int n /*num_blocks=2^n*/, int m /*block_size=2^m words*/, int MEM_READ, int MEM_WRITE, int address = 0, int data_w = 0)
{
	//n=14;
	int MOD = 1 << n;
	unsigned int temp = address >> (m + 2);
	unsigned int Index = temp % MOD;
	unsigned int Tag = temp >> n;
	//m=0;
	int x = 1 << (m + 2); //2^(m+2)=number of bytes in each block
	unsigned int i1 = (address) % x;
	unsigned int i2 = (address + 1) % x;
	unsigned int i3 = (address + 2) % x;
	unsigned int i4 = (address + 3) % x;
	if (MEM_READ > 0)
	{
		if (DMCache[Index].validBit == 1 && DMCache[Index].tag == Tag)
		{
			hits++;
			if (MEM_READ == 1)
				return (int)DMCache[Index].data[i1];
			if (MEM_READ == 2)
			{
				int y = DMCache[Index].data[i1];
				y += (int)(DMCache[Index].data[i2] << 8);
				return y;
			}
			if (MEM_READ == 3)
			{
				int y = DMCache[Index].data[i1];
				y += (int)DMCache[Index].data[i2] << 8;
				y += (int)DMCache[Index].data[i3] << 16;
				y += (int)DMCache[Index].data[i4] << 24;
				return y;
			}
		}
		else
		{
			memoryAccesses++;
			if (DMCache[Index].validBit == 0)
				coldMisses++;
			if (DMCache[Index].validBit == 1 && DMCache[Index].tag != Tag)
				conflictMisses++;
			DMCache[Index].validBit = 1;
			DMCache[Index].tag = Tag;
			for (int i = 0; i < (1 << (m + 2)); i++)
				DMCache[Index].data[(address + i) % x] = memory[address + i];
			if (MEM_READ == 1)
				return memory[address];
			if (MEM_READ == 2)
			{
				int y = memory[address];
				y += (int)memory[address + 1] << 8;
				return y;
			}
			if (MEM_READ == 3)
			{
				int y = memory[address];
				y += (int)memory[address + 1] << 8;
				y += (int)memory[address + 2] << 16;
				y += (int)memory[address + 3] << 24;
				return y;
			}
		}
	}
	else
	{
		if (MEM_WRITE == 1)
		{
			DMCache[Index].validBit = 1;
			DMCache[Index].tag = Tag;
			DMCache[Index].data[i1] = data_w;
			for (int i = 1; i < (1 << (m + 2)); i++)
				DMCache[Index].data[(address + i) % x] = memory[address + i];
			memory[address] = data_w;
		}
		if (MEM_WRITE == 2)
		{
			DMCache[Index].validBit = 1;
			DMCache[Index].tag = Tag;
			DMCache[Index].data[i1] = data_w;
			DMCache[Index].data[i2] = data_w >> 8;
			for (int i = 2; i < (1 << (m + 2)); i++)
				DMCache[Index].data[(address + i) % x] = memory[address + i];
			memory[address] = data_w & ((1 << 8) - 1);
			memory[address + 1] = data_w >> 8;
		}
		if (MEM_WRITE == 3)
		{
			int setb8 = (1 << 8) - 1;
			DMCache[Index].validBit = 1;
			DMCache[Index].tag = Tag;
			DMCache[Index].data[i1] = data_w & setb8;
			DMCache[Index].data[i2] = (data_w & (setb8 << 8)) >> 8;
			DMCache[Index].data[i3] = (data_w & (setb8 << 16)) >> 16;
			DMCache[Index].data[i4] = data_w >> 24;
			memory[address] = data_w & setb8;
			memory[address + 1] = (data_w & (setb8 << 8)) >> 8;
			memory[address + 2] = (data_w & (setb8 << 16)) >> 16;
			memory[address + 3] = data_w >> 24;
		}
	}
	return 0;
}

//pipeline class
class pipelined
{
  public:
	struct Buffer_IF_ID
	{
		unsigned int PC;
		int IR;
		bool en;
		int en2;
		Buffer_IF_ID()
		{
			PC = 0;
			IR = 0;
			en = 0;
			en2 = 1;
		}
	};

	struct Buffer_ID_EX
	{
		unsigned int PC;
		int RA;
		int RB;
		int RZ; // for branch instructions
		int addressC;
		int immediate;
		int ALU_OP, B_SELECT, PC_SELECT, INC_SELECT, Y_SELECT;
		int MEM_READ;
		int MEM_WRITE;
		int RF_WRITE;
		int addressA, addressB;
		unsigned int returnAddress;
		bool branchTaken;
		bool isBranchInstruction;
		bool isLoad;
		bool isStore;
		bool isALU;
		bool isJAL_JALR;

		bool en;
		int en2;
		Buffer_ID_EX()
		{
			en = 0;
			en2 = 1;
			isLoad = FALSE;
			isStore = FALSE;
			isALU = FALSE;
			isJAL_JALR = FALSE;
			branchTaken = FALSE;
			isBranchInstruction = FALSE;
		}
	};

	struct Buffer_EX_MEM
	{
		unsigned int PC;
		int RA;
		int RB;
		int RZ;
		int addressC;
		int immediate;
		int PC_SELECT, INC_SELECT, Y_SELECT;
		int MEM_READ;
		int MEM_WRITE;
		int RF_WRITE;
		bool branchTaken;
		bool isBranchInstruction;
		bool isLoad;
		bool isStore;
		bool isALU;
		bool isJAL_JALR;
		int addressA, addressB;
		unsigned int returnAddress;
		bool en;
		int en2;
		Buffer_EX_MEM()
		{
			isLoad = FALSE;
			isStore = FALSE;
			isALU = FALSE;
			isJAL_JALR = FALSE;
			branchTaken = FALSE;
			isBranchInstruction = FALSE;
			en = 0;
			en2 = 1;
		}
	};

	struct Buffer_MEM_WB
	{
		unsigned int PC;
		int addressC;
		int RF_WRITE;
		int RY;
		bool en;
		int en2;
		bool branchTaken;
		bool isBranchInstruction;
		bool isLoad;
		bool isStore;
		bool isALU;
		bool isJAL_JALR;
		Buffer_MEM_WB()
		{
			isLoad = FALSE;
			isStore = FALSE;
			isALU = FALSE;
			isJAL_JALR = FALSE;
			branchTaken = FALSE;
			isBranchInstruction = FALSE;
			en = 0;
			en2 = 1;
		}
	};

	Buffer_IF_ID buffer_IF_ID;
	Buffer_ID_EX buffer_ID_EX;
	Buffer_EX_MEM buffer_EX_MEM;
	Buffer_MEM_WB buffer_MEM_WB;

	int PC_of_stalledStageEtoE = INT_MAX;
	int PC_of_stalledStageMtoE = INT_MAX;
	int PC_of_stalledStageMtoM = INT_MAX;
	int prevPC = -1;
	/*int prevDependency = -1;
	int lastEtoE = -1;*/

	//Call in decode stage & Writeback Stage
	void readWriteRegFile(int stage)
	{
		if (stage == WB_STAGE)
		{
			if (buffer_MEM_WB.RF_WRITE == 1)
			{
				if (buffer_MEM_WB.addressC)
					regArray[buffer_MEM_WB.addressC] = buffer_MEM_WB.RY;
				return;
			}
		}
		if (stage == DECODE_STAGE)
		{
			if (buffer_ID_EX.addressA < 32)
				buffer_ID_EX.RA = regArray[buffer_ID_EX.addressA];
			if (buffer_ID_EX.addressB < 32)
				buffer_ID_EX.RB = regArray[buffer_ID_EX.addressB];
		}
	}

	/*Instruction Address Generator
    returns returnAddress*/
	lli iag(int stage, int INC_SELECT = 0, int PC_SELECT = 0, lli immediate = 0)
	{
		if (stage == FETCH_STAGE)
		{ //call from fetch
			return buffer_IF_ID.PC + 4;
		}

		if (stage == DECODE_STAGE)
		{ //call from decode
			lli PC_Temp = buffer_ID_EX.PC + 4;
			if (PC_SELECT == 0)
				buffer_ID_EX.PC = buffer_ID_EX.RZ;
			else
			{
				if (INC_SELECT == 1)
					buffer_ID_EX.PC = buffer_ID_EX.PC + immediate;
				else
					buffer_ID_EX.PC = buffer_ID_EX.PC + 4;
			}
			return PC_Temp;
		}
		if (stage == EXECUTE_STAGE)
		{ //call from alu
			lli PC_Temp = buffer_EX_MEM.PC;
			if (PC_SELECT == 0)
				buffer_EX_MEM.PC = buffer_EX_MEM.RZ + 4;
			else
			{
				if (INC_SELECT == 1)
					buffer_EX_MEM.PC = buffer_EX_MEM.PC + immediate;
				else
					buffer_EX_MEM.PC = buffer_EX_MEM.PC + 4;
			}
			return PC_Temp;
		}
	}
	//End of function iag

	//Stage 1: Fetch Stage
	void fetch(bool en)
	{
		buffer_IF_ID.en = en;
		if (en == 0)
		{
			return;
		}

		if (cacheType == 1)
			buffer_IF_ID.IR = readWriteDMCache(DMCache1, n, m, 3, 0, buffer_IF_ID.PC);
		else if (cacheType == 2)
			buffer_IF_ID.IR = readWriteFACache(FACache1, n, m, 3, 0, buffer_IF_ID.PC);
		else if (cacheType == 3)
			buffer_IF_ID.IR = readWriteSACache(SACache1, n, m, k, 3, 0, buffer_IF_ID.PC);
		else
			buffer_IF_ID.IR = readWriteMemory(3, 0, buffer_IF_ID.PC);
		buffer_IF_ID.PC = iag(FETCH_STAGE);
	}
	//End of fetch

	/* Stage 2: Decode Stage
	RA & RB will be updated after this stage */
	void decode()
	{
		if (buffer_IF_ID.en == 0)
			return;
		buffer_ID_EX.isBranchInstruction = FALSE;
		buffer_ID_EX.branchTaken = FALSE;
		buffer_ID_EX.isALU = FALSE;
		buffer_ID_EX.isLoad = FALSE;
		buffer_ID_EX.isStore = FALSE;
		buffer_ID_EX.isJAL_JALR = FALSE;

		int addressA, addressB = 0, addressC;
		int IR = buffer_IF_ID.IR;
		unsigned int PC = buffer_IF_ID.PC;
		unsigned int returnAddress; //Return Address in case of jal/jalr
		int ALU_OP, B_SELECT, PC_SELECT, INC_SELECT, Y_SELECT;
		int MEM_READ;
		int MEM_WRITE;
		int RF_WRITE;
		int immediate; // for immediate values

		unsigned int opcode = IR << 25;
		opcode >>= 25;
		unsigned int funct3 = IR << 17;
		funct3 >>= 29;
		unsigned int funct7 = IR >> 25;
		PC_SELECT = 1;
		INC_SELECT = 0;
		Y_SELECT = 0;
		isLoadInstruction = 0;
		if (opcode == OPCODE_I1)
		{
			buffer_ID_EX.isLoad = TRUE;
			isLoadInstruction = 1;

			RF_WRITE = 1;
			int imm = IR >> 20;
			unsigned int rs1 = IR << 12;
			rs1 >>= 27;
			unsigned int rd = IR << 20;
			rd >>= 27;
			B_SELECT = 1;
			ALU_OP = 0;
			addressA = rs1;
			immediate = imm;
			addressC = rd;
			Y_SELECT = 1;
			if (funct3 == 0)
				MEM_READ = 1;
			else if (funct3 == 1)
				MEM_READ = 2;
			else if (funct3 == 2)
				MEM_READ = 3;
			else if (funct3 == 3)
				MEM_READ = 4;
			MEM_WRITE = 0;
		}

		else if (opcode == OPCODE_I2)
		{
			buffer_ID_EX.isALU = TRUE;

			RF_WRITE = 1;
			int imm = IR >> 20;
			unsigned int rs1 = IR << 12;
			rs1 >>= 27;
			unsigned int rd = IR << 20;
			rd >>= 27;

			B_SELECT = 1;
			addressA = rs1;
			immediate = imm;
			addressC = rd;
			MEM_READ = 0;
			MEM_WRITE = 0;
			if (funct3 == 0) ///addi
			{
				ALU_OP = 0;
			}

			else if (funct3 == 1) //slli
			{
				unsigned int shamt = IR << 7;
				shamt >>= 27;
				immediate = shamt;
				ALU_OP = 7;
			}

			else if (funct3 == 2) //slti
			{
				ALU_OP = 20;
			}

			else if (funct3 == 3) //sltiu
			{
				ALU_OP = 20;
			}

			else if (funct3 == 4) //xori
			{
				ALU_OP = 8;
			}

			else if (funct3 == 5) //srli
			{
				unsigned int shamt = IR << 7;
				shamt >>= 27;
				immediate = shamt;
				if (funct7 == 0)
					ALU_OP = 19;
				else
					ALU_OP = 21;
			}

			else if (funct3 == 6) //ori
			{
				ALU_OP = 6;
			}

			else if (funct3 == 7) //andi
			{
				ALU_OP = 1;
			}
		}

		else if (opcode == OPCODE_I3)
		{
			buffer_ID_EX.isALU = TRUE;

			RF_WRITE = 1;
			addressA = IR << 12;
			addressA >>= 27;
			addressC = IR << 20;
			addressC >>= 27;
			MEM_READ = 0;
			MEM_WRITE = 0;
			unsigned int shamt = IR << 7;
			shamt >>= 27;

			B_SELECT = 1;

			if (funct3 == 0) //addiw
			{
				ALU_OP = 0;
				immediate = IR >> 20;
			}

			else if (funct3 == 1) //slliw
			{
				ALU_OP = 7;
				immediate = shamt;
			}

			else if (funct3 == 5) //srliw,sraiw
			{
				immediate = shamt;
				if (funct7 == 0)
					ALU_OP = 19;
				else
					ALU_OP = 21;
			}
		}

		else if (opcode == OPCODE_I4)
		{ //for jalr
			buffer_ID_EX.isBranchInstruction = TRUE;
			buffer_ID_EX.branchTaken = TRUE;
			buffer_ID_EX.isJAL_JALR = FALSE;

			RF_WRITE = 1;
			int imm = IR >> 20;
			unsigned int rs1 = IR << 12;
			rs1 >>= 27;
			unsigned int rd = IR << 20;
			rd >>= 27;
			PC_SELECT = 0;
			B_SELECT = 1;
			Y_SELECT = 2;
			ALU_OP = 22;
			addressA = rs1;
			immediate = imm;
			addressC = rd;
			MEM_READ = 0;
			MEM_WRITE = 0;
		}

		else if (opcode == OPCODE_S1) //store
		{
			buffer_ID_EX.isStore = TRUE;

			int tmp = (1 << 5) - 1;
			tmp <<= 7;
			int imm1 = IR & tmp;
			imm1 >>= 7;
			int imm2 = IR >> 25;
			immediate = imm1 + (imm2 << 5);

			addressB = IR << 7;
			addressB >>= 27;

			addressA = IR << 12;
			addressA >>= 27;

			if (funct3 == 0)
				MEM_WRITE = 1;
			else if (funct3 == 1)
				MEM_WRITE = 2;
			else if (funct3 == 2)
				MEM_WRITE = 3;
			else if (funct3 == 3)
				MEM_WRITE = 4;
			MEM_READ = 0;
			B_SELECT = 1;
			RF_WRITE = 0;
			ALU_OP = 0;
		}

		else if (opcode == OPCODE_U1) //auipc
		{
			buffer_ID_EX.isALU = TRUE;

			immediate = IR >> 12;

			addressC = IR << 20;
			addressC >>= 27;

			B_SELECT = 1;
			MEM_READ = 0;
			MEM_WRITE = 0;
			RF_WRITE = 1;
			ALU_OP = 12;
		}

		else if (opcode == OPCODE_U2) //lui
		{
			buffer_ID_EX.isALU = TRUE;

			immediate = IR >> 12;

			addressC = IR << 20;
			addressC >>= 27;
			ALU_OP = 13;
			B_SELECT = 1;
			MEM_READ = 0;
			MEM_WRITE = 0;
			RF_WRITE = 1;
		}

		else if (opcode == OPCODE_R1 || opcode == OPCODE_R2)
		{
			buffer_ID_EX.isALU = TRUE;

			unsigned int rs1 = IR << 12;
			rs1 >>= 27;
			unsigned int rs2 = IR << 7;
			rs2 >>= 27;
			unsigned int rd = IR << 20;
			rd >>= 27;
			RF_WRITE = 1;
			B_SELECT = 0;
			if (funct3 == 0)
			{
				if (funct7 == 0)
					ALU_OP = 0; //add,addw
				else if (funct7 == 1)
					ALU_OP = 25; //mul
				else
					ALU_OP = 18; //sub,subw
			}

			else if (funct3 == 1)
				ALU_OP = 7; //sll,sllw

			else if (funct3 == 2)
				ALU_OP = 20; //slt

			else if (funct3 == 3)
				ALU_OP = 20; //sltu

			else if (funct3 == 4)
			{
				if (funct7 == 1)
					ALU_OP = 29; //div
				else
					ALU_OP = 8; //xor
			}

			else if (funct3 == 5)
			{
				if (funct7 == 0)
					ALU_OP = 19; //srl,srlw
				else if (funct7 == 1)
					ALU_OP = 30; // divu
				else
					ALU_OP = 21; //sra,sraw
			}

			else if (funct3 == 6)
			{
				if (funct7 == 1)
					ALU_OP = 31; //rem
				else
					ALU_OP = 6; //or
			}

			else if (funct3 == 7)
			{
				if (funct7 == 1)
					ALU_OP = 32; //remu
				else
					ALU_OP = 1; //and
			}
			addressA = rs1;
			addressB = rs2;
			addressC = rd;
			MEM_READ = 0;
			MEM_WRITE = 0;
		}

		else if (opcode == OPCODE_UJ)
		{ // jal
			buffer_ID_EX.isBranchInstruction = TRUE;
			buffer_ID_EX.branchTaken = TRUE;
			buffer_ID_EX.isJAL_JALR = FALSE;

			unsigned int rd = IR << 20;
			rd >>= 27;
			addressC = rd;
			bitset<20> tmp2(IR >> 12), res;
			for (int i = 18; i >= 11; --i)
				res[i] = tmp2[i - 11];
			res[10] = tmp2[8];
			for (int i = 9; i >= 0; --i)
				res[i] = tmp2[i + 9];
			int tmp1 = res.to_ulong();
			if (tmp2[19])
				tmp1 = tmp1 - (1 << 19);
			immediate = tmp1 * 2;
			RF_WRITE = 1;
			B_SELECT = 0;
			INC_SELECT = 1;
			ALU_OP = -1;
			addressA = 0;
			addressB = 0;
			MEM_READ = 0;
			MEM_WRITE = 0;
			Y_SELECT = 2;
		}

		else if (opcode == OPCODE_SB1)
		{
			buffer_ID_EX.isBranchInstruction = TRUE;

			unsigned int rs1 = IR << 12;
			rs1 >>= 27;
			unsigned int rs2 = IR << 7;
			rs2 >>= 27;
			unsigned int rd = IR << 20;
			rd >>= 27;
			int bit_11 = (rd & 1) << 10;
			int bit_1_4 = rd >> 1;
			int bit_12 = (funct7 >> 6) << 11;
			int bit_5_10 = (funct7 - (bit_12 << 6)) << 4;
			immediate = bit_1_4 | bit_5_10 | bit_11 | bit_12;
			immediate <<= 1;
			RF_WRITE = 0;
			B_SELECT = 0;

			int InA = regArray[rs1];
			int InB = regArray[rs2];

			// Execute moved to decode for stalling
			if (funct3 == 5)
			{ //bge
				ALU_OP = 3;
			}
			else if (funct3 == 7)
			{ //bgeu
				ALU_OP = 34;
			}
			else if (funct3 == 4)
			{ //blt
				ALU_OP = 4;
			}
			else if (funct3 == 6)
			{ //bltu
				ALU_OP = 35;
			}
			else if (funct3 == 0)
			{ //beq
				ALU_OP = 2;
			}
			else if (funct3 == 1) //bne
			{
				ALU_OP = 5;
			}

			addressA = rs1;
			addressB = rs2;
			addressC = 0;
			MEM_READ = 0;
			MEM_WRITE = 0;
		}
		if (addressA < 0)
			addressA += 32;
		if (addressB < 0)
			addressB += 32;
		if (addressC < 0)
			addressC += 32;

		buffer_ID_EX.PC = buffer_IF_ID.PC;

		buffer_ID_EX.addressC = addressC;
		buffer_ID_EX.immediate = immediate;
		buffer_ID_EX.ALU_OP = ALU_OP;
		buffer_ID_EX.B_SELECT = B_SELECT;
		buffer_ID_EX.PC_SELECT = PC_SELECT;
		buffer_ID_EX.INC_SELECT = INC_SELECT;
		buffer_ID_EX.Y_SELECT = Y_SELECT;
		buffer_ID_EX.MEM_READ = MEM_READ;
		buffer_ID_EX.MEM_WRITE = MEM_WRITE;
		buffer_ID_EX.RF_WRITE = RF_WRITE;
		buffer_ID_EX.addressA = addressA;
		buffer_ID_EX.addressB = addressB;
		buffer_ID_EX.returnAddress = returnAddress;
		prevPC = buffer_IF_ID.PC;
		readWriteRegFile(DECODE_STAGE);
	}
	//End of decode

	/* Arithmetic Logic Unit
	Input: ALU_OP, MUXB select, immediate(if any)
	(these input will be provided by decode stage)
	Updates RZ */
	void alu(int ALU_OP, int B_SELECT, int immediate = 0)
	{
		buffer_ID_EX.en = buffer_IF_ID.en;
		if (buffer_ID_EX.en == 0)
			return;

		int INC_SELECT = buffer_ID_EX.INC_SELECT;
		int PC_SELECT = buffer_ID_EX.PC_SELECT;
		unsigned int returnAddress; //Return Address in case of jal/jalr
		immediate = buffer_ID_EX.immediate;

		int RA = buffer_ID_EX.RA;
		buffer_EX_MEM.RA = buffer_ID_EX.RA;
		int RB = buffer_ID_EX.RB;
		buffer_EX_MEM.RB = buffer_ID_EX.RB;
		int RZ;
		buffer_EX_MEM.addressC = buffer_ID_EX.addressC;
		buffer_EX_MEM.immediate = buffer_ID_EX.immediate;
		buffer_EX_MEM.PC_SELECT = buffer_ID_EX.PC_SELECT;
		buffer_EX_MEM.INC_SELECT = buffer_ID_EX.INC_SELECT;
		buffer_EX_MEM.Y_SELECT = buffer_ID_EX.Y_SELECT;
		buffer_EX_MEM.isALU = buffer_ID_EX.isALU;
		buffer_EX_MEM.isLoad = buffer_ID_EX.isLoad;
		buffer_EX_MEM.isStore = buffer_ID_EX.isStore;
		buffer_EX_MEM.isJAL_JALR = buffer_ID_EX.isJAL_JALR;
		buffer_EX_MEM.isBranchInstruction = buffer_ID_EX.isBranchInstruction;
		buffer_EX_MEM.branchTaken = buffer_ID_EX.branchTaken;
		buffer_EX_MEM.MEM_READ = buffer_ID_EX.MEM_READ;
		buffer_EX_MEM.MEM_WRITE = buffer_ID_EX.MEM_WRITE;
		buffer_EX_MEM.RF_WRITE = buffer_ID_EX.RF_WRITE;
		returnAddress = buffer_ID_EX.returnAddress;
		buffer_EX_MEM.addressA = buffer_ID_EX.addressA;
		buffer_EX_MEM.addressB = buffer_ID_EX.addressB;

		buffer_EX_MEM.PC = buffer_ID_EX.PC;

		int InA = RA;
		int InB;
		if (B_SELECT == 0)
			InB = RB;
		else
			InB = immediate;

		if (ALU_OP == 0) //addi,load,
		{
			RZ = InA + InB;
		}

		else if (ALU_OP == 1) //andi
			RZ = InA & InB;

		else if (ALU_OP == 2) //beq
		{
			if (InA == InB)
			{
				INC_SELECT = 1;
				PC_SELECT = 1;
				iag(EXECUTE_STAGE, INC_SELECT, PC_SELECT, immediate);
				buffer_EX_MEM.branchTaken = TRUE;
			}
		}
		else if (ALU_OP == 3) //bge
		{
			if (InA >= InB)
			{
				INC_SELECT = 1;
				PC_SELECT = 1;
				iag(EXECUTE_STAGE, INC_SELECT, PC_SELECT, immediate);
				buffer_EX_MEM.branchTaken = TRUE;
			}
		}

		else if (ALU_OP == 34) //bgeu
		{
			if ((unsigned)InA >= (unsigned)InB)
			{
				INC_SELECT = 1;
				PC_SELECT = 1;
				iag(EXECUTE_STAGE, INC_SELECT, PC_SELECT, immediate);
				buffer_EX_MEM.branchTaken = TRUE;
			}
		}

		else if (ALU_OP == 35) //bltu
		{
			if ((unsigned)InA < (unsigned)InB)
			{
				INC_SELECT = 1;
				PC_SELECT = 1;
				iag(EXECUTE_STAGE, INC_SELECT, PC_SELECT, immediate);
				buffer_EX_MEM.branchTaken = TRUE;
			}
		}

		else if (ALU_OP == 4) //blt
		{
			if (InA < InB)
			{
				INC_SELECT = 1;
				PC_SELECT = 1;
				iag(EXECUTE_STAGE, INC_SELECT, PC_SELECT, immediate);
				buffer_EX_MEM.branchTaken = TRUE;
			}
		}

		else if (ALU_OP == 5) //bne
		{
			if (InA != InB)
			{
				INC_SELECT = 1;
				PC_SELECT = 1;
				iag(EXECUTE_STAGE, INC_SELECT, PC_SELECT, immediate);
				buffer_EX_MEM.branchTaken = TRUE;
			}
		}

		else if (ALU_OP == 6) //ori
			RZ = InA | InB;

		else if (ALU_OP == 7) //slli
			RZ = InA << InB;

		else if (ALU_OP == 18) //sub
			RZ = InA - InB;

		else if (ALU_OP == 8) //xori
			RZ = InA ^ InB;

		else if (ALU_OP == 12) //auipc
		{
			RZ = buffer_ID_EX.PC - 4 + (InB << 12);
		}
		else if (ALU_OP == 13) //lui
			RZ = InB << 12;

		else if (ALU_OP == 19) //srli
			RZ = InA >> InB;

		else if (ALU_OP == 20) //slti,sltiu
			RZ = (InA < InB) ? 1 : 0;

		else if (ALU_OP == 21) //sra, sraw
		{
			RZ = InA >> InB;
			RZ |= InA & (1 << 31);
		}
		else if (ALU_OP == 22) //jalr
		{
			buffer_EX_MEM.RZ = InA + InB;
			RZ = InA + InB;
			returnAddress = iag(EXECUTE_STAGE, INC_SELECT, PC_SELECT, immediate);
		}
		else if (ALU_OP == 25) //mul
			RZ = RA * RB;

		else if (ALU_OP == 29 || ALU_OP == 30) //div, divu
			RZ = RA / RB;

		else if (ALU_OP == 31 || ALU_OP == 32) // rem, remu
			RZ = RA % RB;

		else if (ALU_OP == -1)
		{ // jal
			PC_SELECT = 1;
			returnAddress = iag(EXECUTE_STAGE, INC_SELECT, PC_SELECT, immediate);
		}

		if (buffer_ID_EX.isLoad == TRUE || buffer_ID_EX.isStore == TRUE)
			dataTransferInstructions++;
		else if (buffer_ID_EX.isALU == TRUE)
			aluInstructions++;
		else if (buffer_ID_EX.isBranchInstruction == TRUE)
			controlInstructions++;

		buffer_EX_MEM.RZ = RZ;

		buffer_EX_MEM.returnAddress = returnAddress;
		buffer_EX_MEM.INC_SELECT = INC_SELECT;
		buffer_EX_MEM.PC_SELECT = PC_SELECT;
	}
	//end of ALU function

	/*Stage 4: Memory & RY get updated
    Input: Y_SELECT, MEM_READ, MEM_WRITE, address from RZ/RM, data */
	void memoryStage(int Y_SELECT, int MEM_READ, int MEM_WRITE, int address = 0, int data = 0)
	{
		buffer_EX_MEM.en = buffer_ID_EX.en;
		if (buffer_EX_MEM.en == 0)
			return;
		buffer_MEM_WB.isALU = buffer_EX_MEM.isALU;
		buffer_MEM_WB.isLoad = buffer_EX_MEM.isLoad;
		buffer_MEM_WB.isStore = buffer_EX_MEM.isStore;
		buffer_MEM_WB.isJAL_JALR = buffer_EX_MEM.isJAL_JALR;
		buffer_MEM_WB.isBranchInstruction = buffer_EX_MEM.isBranchInstruction;
		buffer_MEM_WB.branchTaken = buffer_EX_MEM.branchTaken;
		int returnAddress = buffer_EX_MEM.returnAddress;
		int RY;
		int dataFromMem;
		if (cacheType == 1)
			dataFromMem = readWriteDMCache(DMCache2, n, m, MEM_READ, MEM_WRITE, address, data);
		else if (cacheType == 2)
			dataFromMem = readWriteFACache(FACache2, n, m, MEM_READ, MEM_WRITE, address, data);
		else if (cacheType == 3)
			dataFromMem = readWriteSACache(SACache2, n, m, k, MEM_READ, MEM_WRITE, address, data);
		else
			dataFromMem = readWriteMemory(MEM_READ, MEM_WRITE, address, data);
		if (Y_SELECT == 0)
			RY = buffer_EX_MEM.RZ;
		if (Y_SELECT == 1)
			RY = dataFromMem;
		if (Y_SELECT == 2)
			RY = returnAddress;

		buffer_MEM_WB.RY = RY;
		buffer_MEM_WB.addressC = buffer_EX_MEM.addressC;
		buffer_MEM_WB.RF_WRITE = buffer_EX_MEM.RF_WRITE;
		buffer_MEM_WB.PC = buffer_EX_MEM.PC;
	}
	//End of memoryStage

	//Stage 5: WriteBack
	void writeBack(int RF_WRITE, int addressC)
	{
		buffer_MEM_WB.en = buffer_EX_MEM.en;
		if (buffer_MEM_WB.en == 0)
			return;
		readWriteRegFile(WB_STAGE);
	}
	//End of writeBack

	void forward_dependency_EtoE()
	{
		if (buffer_EX_MEM.addressC == 0)
			return;
		if (buffer_EX_MEM.isALU == FALSE && buffer_EX_MEM.isJAL_JALR == FALSE)
			return;

		if (buffer_ID_EX.addressA == buffer_EX_MEM.addressC && buffer_ID_EX.addressB == buffer_EX_MEM.addressC)
		{
			//cout<<"EtoE1"<<endl;
			data_hazard++;
			buffer_ID_EX.RA = buffer_EX_MEM.RZ;
			buffer_ID_EX.RB = buffer_EX_MEM.RZ;
		}
		if (buffer_ID_EX.addressA == buffer_EX_MEM.addressC)
		{
			//cout << "EtoE2" << endl;
			data_hazard++;
			buffer_ID_EX.RA = buffer_EX_MEM.RZ;
			return;
		}
		if (buffer_ID_EX.addressB == buffer_EX_MEM.addressC)
		{
			//cout << "EtoE3" << endl;
			data_hazard++;
			buffer_ID_EX.RB = buffer_EX_MEM.RZ;
			return;
		}
		return;
	}
	//End of forward_dependency_EtoE()

	//Check forward dependency Memory to Execute
	void forward_dependency_MtoE()
	{
		if (buffer_MEM_WB.addressC == 0)
			return;

		if (buffer_ID_EX.addressA == buffer_MEM_WB.addressC && buffer_ID_EX.addressB == buffer_MEM_WB.addressC)
		{
			//cout << "MtoE1" << endl;
			data_hazard++;
			buffer_ID_EX.RA = buffer_MEM_WB.RY;
			buffer_ID_EX.RB = buffer_MEM_WB.RY;
			return;
		}
		if (buffer_ID_EX.addressA == buffer_MEM_WB.addressC)
		{
			//cout << "MtoE2" << endl;
			data_hazard++;
			buffer_ID_EX.RA = buffer_MEM_WB.RY;
			return;
		}
		if (buffer_ID_EX.addressB == buffer_MEM_WB.addressC)
		{
			//cout << "MtoE3" << endl;
			data_hazard++;
			buffer_ID_EX.RB = buffer_MEM_WB.RY;
			return;
		}
		return;
	}
	//End of forward_dependency_MtoE()

	//Check forward dependency Memory to Execute for stalls
	void forward_dependency_MtoEStall()
	{
		if (buffer_MEM_WB.addressC == 0)
			return;
		if (buffer_MEM_WB.isLoad == FALSE)
			return;
		if (buffer_EX_MEM.isStore == TRUE)
			return;
		if (buffer_ID_EX.addressA == buffer_MEM_WB.addressC && buffer_ID_EX.addressB == buffer_MEM_WB.addressC)
		{
			//cout << "MtoEStall1" << endl;
			data_hazard++;
			stalls_data_hazard++;
			cycleCount++;
			buffer_ID_EX.RA = buffer_MEM_WB.RY;
			buffer_ID_EX.RB = buffer_MEM_WB.RY;
			return;
		}
		if (buffer_ID_EX.addressA == buffer_MEM_WB.addressC)
		{
			//cout << "MtoEStall2" << endl;
			data_hazard++;
			stalls_data_hazard++;
			cycleCount++;
			buffer_ID_EX.RA = buffer_MEM_WB.RY;
			return;
		}
		if (buffer_ID_EX.addressB == buffer_MEM_WB.addressC)
		{
			//cout << "MtoEStall3" << endl;
			data_hazard++;
			stalls_data_hazard++;
			cycleCount++;
			buffer_ID_EX.RB = buffer_MEM_WB.RY;
			return;
		}
		return;
	}
	//End of forward_dependency_MtoEStalls()

	//Check data dependency Memory to Memory
	void forward_dependency_MtoM()
	{
		if (buffer_MEM_WB.addressC == 0)
			return;
		if (buffer_MEM_WB.isLoad == FALSE)
			return;

		if (buffer_EX_MEM.isStore == TRUE)
		{ // Load-Store Dependency
			if (buffer_EX_MEM.addressB == buffer_MEM_WB.addressC)
			{
				//cout << "MtoM" << endl;
				data_hazard++;
				buffer_EX_MEM.RB = buffer_MEM_WB.RY;
				//cout<<"Hello"<<buffer_MEM_WB.RY<<endl;
				return;
			}
			return;
		}
	}
	//End of forward_dependency_MtoM()

	//Check data stalling Execute to Execute
	int stall_check_EtoE()
	{
		if (buffer_EX_MEM.addressC == 0)
			return NO_DATA_DEPEND;

		if (buffer_ID_EX.addressA == buffer_EX_MEM.addressC && buffer_ID_EX.addressB == buffer_EX_MEM.addressC)
		{
			PC_of_stalledStageEtoE = buffer_ID_EX.PC;
			return DATA_DEPEND_RA_RB;
		}
		if (buffer_ID_EX.addressA == buffer_EX_MEM.addressC)
		{
			PC_of_stalledStageEtoE = buffer_ID_EX.PC;
			return DATA_DEPEND_RA;
		}
		if (buffer_ID_EX.addressB == buffer_EX_MEM.addressC)
		{
			PC_of_stalledStageEtoE = buffer_ID_EX.PC;
			return DATA_DEPEND_RB;
		}
		return NO_DATA_DEPEND;
	}
	//End of stall_check_EtoE()

	//Check data stalling Memory to Execute
	int stall_check_MtoE()
	{
		if (buffer_MEM_WB.addressC == 0)
			return NO_DATA_DEPEND;

		if (buffer_ID_EX.addressA == buffer_MEM_WB.addressC && buffer_ID_EX.addressB == buffer_MEM_WB.addressC)
		{
			PC_of_stalledStageMtoE = buffer_ID_EX.PC;
			return DATA_DEPEND_RA_RB;
		}

		if (buffer_ID_EX.addressA == buffer_MEM_WB.addressC)
		{
			PC_of_stalledStageMtoE = buffer_ID_EX.PC;
			return DATA_DEPEND_RA;
		}
		if (buffer_ID_EX.addressB == buffer_MEM_WB.addressC)
		{
			PC_of_stalledStageMtoE = buffer_ID_EX.PC;
			return DATA_DEPEND_RB;
		}
		return NO_DATA_DEPEND;
	}
	//End of stall_check_MtoE()

	//Prints pipeline registers data
	void printPipelineRegisters()
	{
		fstream fileWriting;
		fileWriting.open("pipelineRegisters.txt", ios::app);
		fileWriting << "----------------------------------------------------------------------" << endl;
		fileWriting << "Cycle\t:\t" << cycleCount << endl;
		fileWriting << "RA\t\t:\t" << buffer_ID_EX.RA << endl;
		fileWriting << "RB\t\t:\t" << buffer_ID_EX.RB << endl;
		fileWriting << "RZ\t\t:\t" << buffer_EX_MEM.RZ << endl;
		fileWriting << "RY\t\t:\t" << buffer_MEM_WB.RY << endl;
		fileWriting << "----------------------------------------------------------------------" << endl;
		fileWriting.close();
	}
	//End of printpipelineRegisters

	//Run Instructions: pipelined
	void runCode()
	{
		bool en = 1;
		int branchDecisionMade = 0;
		int instruction_number; //for knob5
		cout << "Knob 2 (Data Forwarding Knob)           :  ";
		cin >> knob2;
		cout << "Knob 3 (Register File Knob)             :  ";
		cin >> knob3;
		cout << "Knob 4 (Pipeline Register Knob)         :  ";
		cin >> knob4;
		cout << "Knob 5 (Instruction Knob)               :  ";
		cin >> knob5;
		if (knob5 == ON)
		{
			cout << "Instruction Number                      :  ";
			cin >> instruction_number;
			instruction_number = instruction_number << 2;
		}
		cout << "----------------------------------------------------------------------" << endl;
		int tmp3 = -1;
		int tmp2 = 0;
		int flag = 0;
		while (1)
		{
			cycleCount++;
			if (memory[buffer_IF_ID.PC] == 0 && memory[buffer_IF_ID.PC + 1] == 0 && memory[buffer_IF_ID.PC + 2] == 0 && memory[buffer_IF_ID.PC + 3] == 0)
				en = 0;

			if (knob2 == 0)
			{

				if (buffer_MEM_WB.en2 == 1)
				{

					writeBack(buffer_MEM_WB.RF_WRITE, buffer_MEM_WB.addressC);
				}

				if (buffer_EX_MEM.en2 == 1)
				{

					buffer_MEM_WB.en2 = 1;
					memoryStage(buffer_EX_MEM.Y_SELECT, buffer_EX_MEM.MEM_READ, buffer_EX_MEM.MEM_WRITE, buffer_EX_MEM.RZ, buffer_EX_MEM.RB);
				}
				else
				{
					buffer_MEM_WB.en2 = 0;
					buffer_MEM_WB.addressC = 0;
				}

				if (buffer_ID_EX.en2 == 1)
				{
					buffer_EX_MEM.en2 = 1;
					alu(buffer_ID_EX.ALU_OP, buffer_ID_EX.B_SELECT, buffer_ID_EX.immediate);
				}
				else
				{
					buffer_EX_MEM.en2 = 0;
					buffer_EX_MEM.addressC = 0;
				}

				if (buffer_IF_ID.en2 < 1)
					buffer_IF_ID.en2++;

				if (buffer_IF_ID.en2 == 1 && branchDecisionMade == 0)
				{

					buffer_ID_EX.en2 = 1;
					decode();
				}
				else
				{
					buffer_ID_EX.en2 = 0;
				}
				int dataDependencyMtoE = stall_check_MtoE();
				//cout<<buffer_IF_ID.en2<<" OUT"<<cycleCount<<endl;
				if (dataDependencyMtoE != NO_DATA_DEPEND)
				{
					buffer_IF_ID.en2 = 0;
					buffer_ID_EX.en2 = 0;
					stalls_data_hazard+=1;
					data_hazard++;
				}
				//cout << buffer_IF_ID.en2 << " OUT2" <<cycleCount<< endl;
				int dataDependencyEtoE = stall_check_EtoE();
				if (dataDependencyEtoE != NO_DATA_DEPEND)
				{	
					buffer_IF_ID.en2 = -1;
					buffer_ID_EX.en2 = 0;
					stalls_data_hazard++;
					tmp2=1;
					data_hazard++;
					//cout<<cycleCount<<" PPP"<<endl;
				}
				if(dataDependencyEtoE != NO_DATA_DEPEND && tmp2==1)
				{   
					data_hazard--;
					tmp2=0;
				}
				//cout<<cycleCount<< " RRRR"<<endl;
			//	if (dataDependencyMtoE != NO_DATA_DEPEND && tmp2 != NO_DATA_DEPEND)
			//	{  
			//		stalls_data_hazard--;
			//		data_hazard--;
		//			tmp2=NO_DATA_DEPEND;
		//		}
				if (buffer_IF_ID.en2 == 1)
				{

					fetch(en);
					branchDecisionMade = 0;
				}

				if (buffer_EX_MEM.isBranchInstruction == TRUE && buffer_EX_MEM.branchTaken == TRUE && branchDecisionMade == 0)
				{
					buffer_IF_ID.PC = buffer_EX_MEM.PC - 4;
					buffer_IF_ID.en2 = 0;
					buffer_ID_EX.en2 = 0;
					buffer_EX_MEM.isBranchInstruction = FALSE;
					buffer_EX_MEM.branchTaken = FALSE;
					stalls_control_hazard += 2;
					branch_mispredictions++;
					branchDecisionMade = 1;
				}
			}
			else if (knob2 == 1)
			{
				if (buffer_MEM_WB.en2 == 1)
				{
					writeBack(buffer_MEM_WB.RF_WRITE, buffer_MEM_WB.addressC);
				}
				forward_dependency_MtoE();
				forward_dependency_MtoM();
				forward_dependency_EtoE();

				if (buffer_EX_MEM.en2 == 1)
				{
					memoryStage(buffer_EX_MEM.Y_SELECT, buffer_EX_MEM.MEM_READ, buffer_EX_MEM.MEM_WRITE, buffer_EX_MEM.RZ, buffer_EX_MEM.RB);
					buffer_MEM_WB.en2 = 1;
				}
				else
					buffer_MEM_WB.en2 = 0;

				forward_dependency_MtoEStall();

				if (buffer_ID_EX.en2 == 1)
				{
					alu(buffer_ID_EX.ALU_OP, buffer_ID_EX.B_SELECT, buffer_ID_EX.immediate);
					buffer_EX_MEM.en2 = 1;
				}
				else
					buffer_EX_MEM.en2 = 0;

				if (buffer_IF_ID.en2 == 1)
				{
					decode();
					buffer_ID_EX.en2 = 1;
				}
				else
					buffer_ID_EX.en2 = 0;

				fetch(en);
				buffer_IF_ID.en2 = 1;

				if (buffer_EX_MEM.isBranchInstruction == TRUE && buffer_EX_MEM.branchTaken == TRUE)
				{
					buffer_IF_ID.PC = buffer_EX_MEM.PC - 4;
					buffer_IF_ID.en2 = 0;
					buffer_ID_EX.en2 = 0;
					buffer_EX_MEM.isBranchInstruction = FALSE;
					buffer_EX_MEM.branchTaken = FALSE;
					stalls_control_hazard += 2;
					branch_mispredictions++;
				}
			}
			if (knob3 == ON)
				printRegisterFile();

			if (knob4 == ON)
				printPipelineRegisters();

			if (knob5 == ON)
			{
				if (buffer_ID_EX.PC == instruction_number)
				{
					cout << "Cycle Number\t:\t" << cycleCount << endl;
					cout << "RA\t\t:\t" << buffer_ID_EX.RA << endl;
					cout << "RB\t\t:\t" << buffer_ID_EX.RB << endl;
				}
				if (buffer_EX_MEM.PC == instruction_number)
				{
					cout << "Cycle Number\t:\t" << cycleCount << endl;
					cout << "RZ\t\t:\t" << buffer_EX_MEM.RZ << endl;
				}
				if (buffer_MEM_WB.PC == instruction_number)
				{
					cout << "Cycle Number\t:\t" << cycleCount << endl;
					cout << "RY\t\t:\t" << buffer_MEM_WB.RY << endl;
				}
			}

			if (en == 0 && buffer_IF_ID.en == 0 && buffer_ID_EX.en == 0 && buffer_EX_MEM.en == 0 && buffer_MEM_WB.en == 0)
				break;

			if (knob2 == ON)
				cout << data_hazard;
		}
		// Inserted NOP

		if(knob2==ON)
			data_hazard+=stalls_data_hazard;
		cycleCount -= 2;
		aluInstructions -= 1;
		if (knob3 == OFF)
			printRegisterFile();
		cout << "----------------------------------------------------------------------" << endl;
	}
	//End of runCode
};

class unpipelined
{
  public:
	unsigned int PC = 0;	//Program Counter
	int IR;					//Instruction Register
	int RA, RB, RZ, RY, RM; //Interstage Buffers
	unsigned int addressA, addressB;
	int immediate;				// for immediate values
	int addressC;				//destination register
	unsigned int returnAddress; //Return Address in case of jal/jalr

	//Control Signals
	int ALU_OP, B_SELECT, PC_SELECT, INC_SELECT, Y_SELECT;
	int MEM_READ;
	int MEM_WRITE;
	int RF_WRITE;

	//Call in decode stage & Writeback Stage
	void readWriteRegFile(int RF_WRITE, int addressA, int addressB, int addressC)
	{
		if (RF_WRITE == 1)
		{
			if (addressC)
				regArray[addressC] = RY;
			return;
		}

		RA = regArray[addressA];
		RB = regArray[addressB];
	}
	//End of readWriteRegFile

	/*Instruction Address Generator
    returns returnAddress*/
	lli iag(int INC_SELECT, int PC_SELECT, lli immediate = 0)
	{
		lli PC_Temp = PC + 4;
		if (PC_SELECT == 0)
			PC = RZ;
		else
		{
			if (INC_SELECT == 1)
				PC = PC + immediate;
			else
				PC = PC + 4;
		}
		return PC_Temp;
	}
	//End of function iag

	//Stage 1: Fetch Stage
	void fetch()
	{
		if (cacheType == 1)
			IR = readWriteDMCache(DMCache1, n, m, 3, 0, PC);
		else if (cacheType == 2)
			IR = readWriteFACache(FACache1, n, m, 3, 0, PC);
		else if (cacheType == 3)
			IR = readWriteSACache(SACache1, n, m, k, 3, 0, PC);
		else
			IR = readWriteMemory(3, 0, PC);
		returnAddress = iag(0, 1);
	}
	//end of fetch

	/* Stage 2: Decode Stage
    RA & RB will be updated after this stage */
	void decode()
	{

		unsigned int opcode = IR << 25;
		opcode >>= 25;
		unsigned int funct3 = IR << 17;
		funct3 >>= 29;
		unsigned int funct7 = IR >> 25;
		PC_SELECT = 1;
		INC_SELECT = 0;
		Y_SELECT = 0;
		if (opcode == OPCODE_I1)
		{
			//stats_count.ITypeInstructions++;
			dataTransferInstructions++;
			RF_WRITE = 1;
			int imm = IR >> 20;
			unsigned int rs1 = IR << 12;
			rs1 >>= 27;
			unsigned int rd = IR << 20;
			rd >>= 27;
			B_SELECT = 1;
			ALU_OP = 0;
			addressA = rs1;
			immediate = imm;
			addressC = rd;
			Y_SELECT = 1;
			if (funct3 == 0)
				MEM_READ = 1;
			else if (funct3 == 1)
				MEM_READ = 2;
			else if (funct3 == 2)
				MEM_READ = 3;
			else if (funct3 == 3)
				MEM_READ = 4;
			MEM_WRITE = 0;
		}

		else if (opcode == OPCODE_I2)
		{

			//stats_count.ITypeInstructions++;
			aluInstructions++;
			RF_WRITE = 1;
			int imm = IR >> 20;
			unsigned int rs1 = IR << 12;
			rs1 >>= 27;
			unsigned int rd = IR << 20;
			rd >>= 27;

			B_SELECT = 1;
			addressA = rs1;
			immediate = imm;
			addressC = rd;
			MEM_READ = 0;
			MEM_WRITE = 0;
			if (funct3 == 0) ///addi
			{
				ALU_OP = 0;
			}

			else if (funct3 == 1) //slli
			{
				unsigned int shamt = IR << 7;
				shamt >>= 27;
				immediate = shamt;
				ALU_OP = 7;
			}

			else if (funct3 == 2) //slti
			{
				ALU_OP = 20;
			}

			else if (funct3 == 3) //sltiu
			{
				ALU_OP = 20;
			}

			else if (funct3 == 4) //xori
			{
				ALU_OP = 8;
			}

			else if (funct3 == 5) //srli
			{
				unsigned int shamt = IR << 7;
				shamt >>= 27;
				immediate = shamt;
				if (funct7 == 0)
					ALU_OP = 19;
				else
					ALU_OP = 21;
			}

			else if (funct3 == 6) //ori
			{
				ALU_OP = 6;
			}

			else if (funct3 == 7) //andi
			{
				ALU_OP = 1;
			}
		}

		else if (opcode == OPCODE_I3)
		{
			//stats_count.ITypeInstructions++;
			aluInstructions++;
			RF_WRITE = 1;
			addressA = IR << 12;
			addressA >>= 27;
			addressC = IR << 20;
			addressC >>= 27;
			MEM_READ = 0;
			MEM_WRITE = 0;
			unsigned int shamt = IR << 7;
			shamt >>= 27;

			B_SELECT = 1;

			if (funct3 == 0) //addiw
			{
				ALU_OP = 0;
				immediate = IR >> 20;
			}

			else if (funct3 == 1) //slliw
			{
				ALU_OP = 7;
				immediate = shamt;
			}

			else if (funct3 == 5) //srliw,sraiw
			{
				immediate = shamt;
				if (funct7 == 0)
					ALU_OP = 19;
				else
					ALU_OP = 21;
			}
		}

		else if (opcode == OPCODE_I4)
		{ //for jalr
			//stats_count.ITypeInstructions++;
			controlInstructions++;
			RF_WRITE = 1;
			int imm = IR >> 20;
			unsigned int rs1 = IR << 12;
			rs1 >>= 27;
			unsigned int rd = IR << 20;
			rd >>= 27;
			PC_SELECT = 0;
			B_SELECT = 1;
			Y_SELECT = 2;
			ALU_OP = 22;
			addressA = rs1;
			immediate = imm;
			addressC = rd;
			MEM_READ = 0;
			MEM_WRITE = 0;
		}

		else if (opcode == OPCODE_S1) //store
		{
			//stats_count.STypeInstructions++;
			dataTransferInstructions++;

			int tmp = (1 << 5) - 1;
			tmp <<= 7;
			int imm1 = IR & tmp;
			imm1 >>= 7;
			int imm2 = IR >> 25;
			immediate = imm1 + (imm2 << 5);

			addressB = IR << 7;
			addressB >>= 27;

			addressA = IR << 12;
			addressA >>= 27;
			if (funct3 == 0)
				MEM_WRITE = 1;
			else if (funct3 == 1)
				MEM_WRITE = 2;
			else if (funct3 == 2)
				MEM_WRITE = 3;
			else if (funct3 == 3)
				MEM_WRITE = 4;
			MEM_READ = 0;
			B_SELECT = 1;
			RF_WRITE = 0;
			ALU_OP = 0;
		}

		else if (opcode == OPCODE_U1) //auipc
		{
			//stats_count.UTypeInstructions++;
			aluInstructions++;
			immediate = IR >> 12;

			addressC = IR << 20;
			addressC >>= 27;

			B_SELECT = 1;
			MEM_READ = 0;
			MEM_WRITE = 0;
			RF_WRITE = 1;
			ALU_OP = 12;
		}

		else if (opcode == OPCODE_U2) //lui
		{
			//stats_count.UTypeInstructions++;
			aluInstructions++;
			immediate = IR >> 12;

			addressC = IR << 20;
			addressC >>= 27;
			ALU_OP = 13;
			B_SELECT = 1;
			MEM_READ = 0;
			MEM_WRITE = 0;
			RF_WRITE = 1;
		}

		else if (opcode == OPCODE_R1 || opcode == OPCODE_R2)
		{
			//stats_count.RTypeInstructions++;
			aluInstructions++;
			unsigned int rs1 = IR << 12;
			rs1 >>= 27;
			unsigned int rs2 = IR << 7;
			rs2 >>= 27;
			unsigned int rd = IR << 20;
			rd >>= 27;
			RF_WRITE = 1;
			B_SELECT = 0;
			if (funct3 == 0)
			{
				if (funct7 == 0)
					ALU_OP = 0; //add,addw
				else if (funct7 == 1)
					ALU_OP = 25; //mul
				else
					ALU_OP = 18; //sub,subw
			}

			else if (funct3 == 1)
				ALU_OP = 7; //sll,sllw

			else if (funct3 == 2)
				ALU_OP = 20; //slt

			else if (funct3 == 3)
				ALU_OP = 20; //sltu

			else if (funct3 == 4)
			{
				if (funct7 == 1)
					ALU_OP = 29; //div
				else
					ALU_OP = 8; //xor
			}

			else if (funct3 == 5)
			{
				if (funct7 == 0)
					ALU_OP = 19; //srl,srlw
				else if (funct7 == 1)
					ALU_OP = 30; // divu
				else
					ALU_OP = 21; //sra,sraw
			}

			else if (funct3 == 6)
			{
				if (funct7 == 1)
					ALU_OP = 31; //rem
				else
					ALU_OP = 6; //or
			}

			else if (funct3 == 7)
			{
				if (funct7 == 1)
					ALU_OP = 32; //remu
				else
					ALU_OP = 1; //and
			}
			addressA = rs1;
			addressB = rs2;
			addressC = rd;
			MEM_READ = 0;
			MEM_WRITE = 0;
		}

		else if (opcode == OPCODE_UJ)
		{
			//stats_count.UJTypeInstructions++;
			controlInstructions++;
			unsigned int rd = IR << 20;
			rd >>= 27;
			addressC = rd;
			bitset<20> tmp2(IR >> 12), res;
			for (int i = 18; i >= 11; --i)
				res[i] = tmp2[i - 11];
			res[10] = tmp2[8];
			for (int i = 9; i >= 0; --i)
				res[i] = tmp2[i + 9];
			int tmp1 = res.to_ulong();
			if (tmp2[19])
				tmp1 = tmp1 - (1 << 19);
			immediate = tmp1 * 2;
			RF_WRITE = 1;
			B_SELECT = 0;
			INC_SELECT = 1;
			ALU_OP = -1;
			addressA = 0;
			addressB = 0;
			MEM_READ = 0;
			MEM_WRITE = 0;
			Y_SELECT = 2;
		}

		else if (opcode == OPCODE_SB1)
		{
			//stats_count.SBTypeInstructions++;
			controlInstructions++;
			unsigned int rs1 = IR << 12;
			rs1 >>= 27;
			unsigned int rs2 = IR << 7;
			rs2 >>= 27;
			unsigned int rd = IR << 20;
			rd >>= 27;
			int bit_11 = (rd & 1) << 10;
			int bit_1_4 = rd >> 1;
			int bit_12 = (funct7 >> 6) << 11;
			int bit_5_10 = (funct7 - (bit_12 << 6)) << 4;
			immediate = bit_1_4 | bit_5_10 | bit_11 | bit_12;
			immediate <<= 1;

			RF_WRITE = 0;
			B_SELECT = 0;
			if (funct3 == 5) //bge
				ALU_OP = 3;
			else if (funct3 == 7)
			{ //bgeu
				ALU_OP = 34;
			}
			else if (funct3 == 4) //blt
				ALU_OP = 4;

			else if (funct3 == 6)
			{ //bltu
				ALU_OP = 35;
			}
			else if (funct3 == 0) //beq
				ALU_OP = 2;
			else if (funct3 == 1) //bne Update task2.cpp
				ALU_OP = 5;
			addressA = rs1;
			addressB = rs2;
			addressC = 0;
			MEM_READ = 0;
			MEM_WRITE = 0;
		}
		readWriteRegFile(0, addressA, addressB, addressC);
	}
	//End of decode

	/* Arithmetic Logic Unit
    Input: ALU_OP, MUXB select, immediate(if any)
    (these input will be provided by decode stage)
    Updates RZ */
	void alu(int ALU_OP, int B_SELECT, int immediate = 0)
	{
		int InA = RA;
		int InB;
		if (B_SELECT == 0)
			InB = RB;
		else
			InB = immediate;

		if (ALU_OP == 0) //addi,load,
		{
			RZ = InA + InB;
		}

		else if (ALU_OP == 1) //andi
			RZ = InA & InB;

		else if (ALU_OP == 2) //beq
		{
			if (InA == InB)
			{
				INC_SELECT = 1;
				PC -= 4;
				iag(INC_SELECT, 1, immediate);
			}
		}
		else if (ALU_OP == 3) //bge
		{
			if (InA >= InB)
			{
				INC_SELECT = 1;
				PC -= 4;
				iag(INC_SELECT, 1, immediate);
			}
		}

		//bgeu
		else if (ALU_OP == 34)
		{
			if ((unsigned)InA >= (unsigned)InB)
			{
				INC_SELECT = 1;
				PC -= 4;
				iag(INC_SELECT, 1, immediate);
			}
		}

		//bltu
		else if (ALU_OP == 35)
		{
			if ((unsigned)InA < (unsigned)InB)
			{
				INC_SELECT = 1;
				PC -= 4;
				iag(INC_SELECT, 1, immediate);
			}
		}

		else if (ALU_OP == 4) //blt
		{
			if (InA < InB)
			{
				INC_SELECT = 1;
				PC -= 4;
				iag(INC_SELECT, 1, immediate);
			}
		}

		else if (ALU_OP == 5) //bne
		{
			if (InA != InB)
			{
				INC_SELECT = 1;
				PC -= 4;
				iag(INC_SELECT, 1, immediate);
			}
		}

		else if (ALU_OP == 6) //ori
			RZ = InA | InB;

		else if (ALU_OP == 7) //slli
			RZ = InA << InB;

		else if (ALU_OP == 18) //sub
			RZ = InA - InB;

		else if (ALU_OP == 8) //xori
			RZ = InA ^ InB;

		else if (ALU_OP == 12) //auipc
		{
			RZ = PC - 4 + (InB << 12);
		}
		else if (ALU_OP == 13) //lui
			RZ = InB << 12;

		else if (ALU_OP == 19) //srli
			RZ = InA >> InB;

		else if (ALU_OP == 20) //slti,sltiu
			RZ = (InA < InB) ? 1 : 0;

		else if (ALU_OP == 21) //sra, sraw
		{
			RZ = InA >> InB;
			RZ |= InA & (1 << 31);
		}
		else if (ALU_OP == 22) //jalr
		{
			RZ = InA + InB;
			PC -= 4;
			returnAddress = iag(0, PC_SELECT, immediate);
		}
		else if (ALU_OP == 25) //mul
			RZ = RA * RB;

		else if (ALU_OP == 29 || ALU_OP == 30) //div, divu
			RZ = RA / RB;

		else if (ALU_OP == 31 || ALU_OP == 32) // rem, remu
			RZ = RA % RB;

		else if (ALU_OP == -1)
		{ // jal
			PC -= 4;
			returnAddress = iag(INC_SELECT, 1, immediate);
		}
	}
	//end of ALU function

	/*Stage 4: Memory & RY get updated
    Input: Y_SELECT, MEM_READ, MEM_WRITE, address from RZ/RM, data */
	void memoryStage(int Y_SELECT, int MEM_READ, int MEM_WRITE, int address = 0, int data = 0)
	{
		int dataFromMem;
		if (cacheType == 1)
			dataFromMem = readWriteDMCache(DMCache2, n, m, MEM_READ, MEM_WRITE, address, data);
		else if (cacheType == 2)
			dataFromMem = readWriteFACache(FACache2, n, m, MEM_READ, MEM_WRITE, address, data);
		else if (cacheType == 3)
			dataFromMem = readWriteSACache(SACache2, n, m, k, MEM_READ, MEM_WRITE, address, data);
		else
			dataFromMem = readWriteMemory(MEM_READ, MEM_WRITE, address, data);

		if (Y_SELECT == 0)
			RY = RZ;
		if (Y_SELECT == 1)
			RY = dataFromMem;
		if (Y_SELECT == 2)
			RY = returnAddress;
	}
	//End of memoryStage

	//Stage 5: WriteBack
	void writeBack(int RF_WRITE, int addressC)
	{
		readWriteRegFile(RF_WRITE, 0, 0, addressC);
	}
	//End of writeBack

	void printPipelineRegisters()
	{
		fstream fileWriting;
		fileWriting.open("pipelineRegisters.txt", ios::app);
		fileWriting << "----------------------------------------------------------------------" << endl;
		fileWriting << "Cycle\t:\t" << cycleCount << endl;
		fileWriting << "RA\t\t:\t" << RA << endl;
		fileWriting << "RB\t\t:\t" << RB << endl;
		fileWriting << "RZ\t\t:\t" << RZ << endl;
		fileWriting << "RY\t\t:\t" << RY << endl;
		fileWriting << "----------------------------------------------------------------------" << endl;
		fileWriting.close();
	}
	//End of printpipelineRegisters

	//Run Instructions: unpipelined
	void runCode()
	{
		fstream fileWriting;
		fileWriting.open("pipelineRegisters.txt", ios::out); //To clear data of existing pipelineRegisters.txt
		fileWriting.close();

		int instruction_number; //for knob5
		cout << "Knob 3 (Register File Knob)             :  ";
		cin >> knob3;
		cout << "Knob 4 (Pipeline Register Knob)         :  ";
		cin >> knob4;
		cout << "Knob 5 (Instruction Knob)               :  ";
		cin >> knob5;
		if (knob5 == ON)
		{
			cout << "Instruction Number                      :  ";
			cin >> instruction_number;
			instruction_number = instruction_number << 2;
		}
		cout << "----------------------------------------------------------------------" << endl;

		while (1)
		{
			if (memory[PC] == 0 && memory[PC + 1] == 0 && memory[PC + 2] == 0 && memory[PC + 3] == 0)
				break;
			fetch();
			decode();
			alu(ALU_OP, B_SELECT, immediate);
			memoryStage(Y_SELECT, MEM_READ, MEM_WRITE, RZ, RB);
			writeBack(RF_WRITE, addressC);
			cycleCount++;

			if (knob3 == ON)
				printRegisterFile();
			if (knob4 == ON)
				printPipelineRegisters();
			if (knob5 == ON)
			{
				if (PC == instruction_number)
				{
					cout << "Cycle Number\t:\t" << cycleCount << endl;
					cout << "RA\t\t:\t" << RA << endl;
					cout << "RB\t\t:\t" << RB << endl;
					cout << "RZ\t\t:\t" << RZ << endl;
					cout << "RY\t\t:\t" << RY << endl;
				}
			}
		}
		if (knob3 == OFF)
			printRegisterFile();
		cout << "----------------------------------------------------------------------" << endl;
	}
	//End of runCode
};

//start of selectCache
void selectCache()
{
	cout << "----------------------------------------------------------------------" << endl;
	cout << "Press 1 : Direct Mapped Cache" << endl;
	cout << "Press 2 : Fully Associative Cache" << endl;
	cout << "Press 3 : Set Associative Cache" << endl;
	cout << "Press 4 : Without Cache Implementation" << endl;
	cout << "----------------------------------------------------------------------" << endl;
	cout << "Knob 0 (Cache Type Knob)                :  ";
	cin >> cacheType;
	if (cacheType == 1)
	{
		int x, y;
		cout << "Enter X (2^X - cache size in bytes)     :  ";
		cin >> x;
		cout << "Enter BLOCK SIZE (in words)             :  ";
		cin >> y;
		n = x - y - 2;
		m = y;
		DMCache1 = new dmCache[1 << n];
		DMCache2 = new dmCache[1 << n];
		for (int i = 0; i < (1 << n); i++)
		{
			DMCache1[i].data = new unsigned char[1 << (m + 2)]; //instruction cache
			DMCache2[i].data = new unsigned char[1 << (m + 2)]; //data cache
		}
	}
	else if (cacheType == 2)
	{
		int x, y;
		cout << "Enter X (2^X - cache size in bytes)     :  ";
		cin >> x;
		cout << "Enter BLOCK SIZE (in words)             :  ";
		cin >> y;
		n = x - y - 2;
		m = y;
		FACache1 = new faCache[1 << n];
		FACache2 = new faCache[1 << n];
		for (int i = 0; i < (1 << n); i++)
		{
			FACache1[i].data = new unsigned char[1 << (m + 2)];
			FACache2[i].data = new unsigned char[1 << (m + 2)];
		}
	}
	else if (cacheType == 3)
	{
		int x, y;
		cout << "Enter X (2^X - cache size in bytes)     :  ";
		cin >> x;
		cout << "Enter BLOCK SIZE (in words)             :  ";
		cin >> y;
		cout << "Enter K (2^K-way associative cache)     :  ";
		cin >> k;
		n = x - k - y - 2;
		m = y;
		k = 1 << k;
		SACache1 = new saCache *[1 << n];
		SACache2 = new saCache *[1 << n];
		for (int i = 0; i < (1 << n); ++i)
		{
			SACache1[i] = new saCache[k];
			SACache2[i] = new saCache[k];
		}
		for (int i = 0; i < (1 << n); i++)
		{
			for (int j = 0; j < k; j++)
			{
				SACache1[i][j].data = new unsigned char[1 << (m + 2)];
				SACache2[i][j].data = new unsigned char[1 << (m + 2)];
			}
		}
	}
}
//End of selectCache

//Update memory with data & instructions
void updateMemory()
{
	string machineLine;
	string machineCode;
	fstream fileReading;

	map<char, int> hexadecimal;
	for (int i = 0; i <= 9; ++i)
		hexadecimal[i + '0'] = i;
	for (int i = 0; i <= 6; ++i)
		hexadecimal[i + 'A'] = i + 10;

	fileReading.open("machineData.txt");
	while (getline(fileReading, machineLine))
	{
		lli value = 0, address = 0;
		string type = "";
		int i = 0;
		while (machineLine[i] != ' ')
			value = value * 10 + (machineLine[i++] - '0');
		i = i + 3;
		while (machineLine[i] != ' ')
			address = address * 16 + hexadecimal[machineLine[i++]];
		i++;

		while (i < machineLine.length() && machineLine[i] != ' ')
			type += machineLine[i++];
		if (type == "byte")
			readWriteMemory(0, 1, address, value);
		else if (type == "halfword")
			readWriteMemory(0, 2, address, value);
		else if (type == "word")
			readWriteMemory(0, 3, address, value);
		else if (type == "doubleword")
			readWriteMemory(0, 4, address, value);
	}
	fileReading.close();

	fileReading.open("machineCode.mc");
	lli address = 0;
	while (getline(fileReading, machineLine))
	{
		lli value = 0;

		int i = 2;
		while (i < machineLine.length())
			value = value * 16 + hexadecimal[machineLine[i++]];
		if (cacheType == 1)
			readWriteDMCache(DMCache1, n, m, 0, 3, address, value);
		if (cacheType == 2)
			readWriteFACache(FACache1, n, m, 0, 3, address, value);
		if (cacheType == 3)
			readWriteSACache(SACache1, n, m, 0, 3, address, value);
		else
			readWriteMemory(0, 3, address, value);
		address += 4;
	}
	if (knob1 == ON)
	{
		lli value = 0x00000033;
		for (int i = 0; i < 2; i++)
		{
			if (cacheType == 1)
				readWriteDMCache(DMCache1, n, m, 0, 3, address, value);
			if (cacheType == 2)
				readWriteFACache(FACache1, n, m, 0, 3, address, value);
			if (cacheType == 3)
				readWriteSACache(SACache1, n, m, 0, 3, address, value);
			else
				readWriteMemory(0, 3, address, value);
			address += 4;
		}
	}
	fileReading.close();
}
//End of updateMemory

//main function
int main()
{
	regArray[2] = 0xFFFFFF; //initialize x2, x3
	regArray[3] = 0x100000;

	fstream fileWriting; //To clear data of existing regFile.txt
	fileWriting.open("regFile.txt", ios::out);
	fileWriting.close();
	selectCache();
	cout << "----------------------------------------------------------------------" << endl;
	cout << "Press 0 : Turn OFF the KNOB" << endl;
	cout << "Press 1 : Turn ON the KNOB" << endl;
	cout << "----------------------------------------------------------------------" << endl;
	cout << "Knob 1 (Pipeline Knob)                  :  ";
	cin >> knob1;
	updateMemory(); //Update memory with data & instructions
	if (knob1 == ON)
	{
		pipelined execute_pipeline;
		execute_pipeline.runCode();
	}
	else
	{
		unpipelined execute_unpipeline;
		execute_unpipeline.runCode();
	}
	stats_print();
	printMemory();
}
//End of main
