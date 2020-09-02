//Task 1: Assembly Code to Machine Code
#include <bits/stdc++.h>
#include<unordered_map>
#define lli long long int
using namespace std;

//User defined data type: to store line no of each label
struct labelData{
	string label;
	lli lineNumber;
};

//to store variables and addresses defined in .data segment
struct data{
	string var;
	string hexaddress;
};

//Function to convert integer number to binary string 
string dec2Binary(lli decimalNum, int length){
	int i=0,l=length;
	lli positiveDecimalNum=abs(decimalNum);
	string binaryNum="";
	while(l--)
		binaryNum=binaryNum+"0";
	while(positiveDecimalNum>0){
       		binaryNum[length-i-1]=positiveDecimalNum%2+'0'; 
       		positiveDecimalNum=positiveDecimalNum/2; 
       		i++; 
    	}
	if(decimalNum<0){
    		for(int j=0;j<length;j++)
    			if(binaryNum[j]=='0')
    				binaryNum[j]='1';
    			else
    				binaryNum[j]='0';
    		int carry=1;
    		for(int j=length-1;j>=0;j--){
    			if(carry==1 && binaryNum[j]=='0'){
    				binaryNum[j]='1';
				break;
			}
    			else if(carry==1 && binaryNum[j]=='1')
    				binaryNum[j]='0';
		}
	}
    return binaryNum;
}
//End of function dec2Binary

//Function to create map between binary number and its equivalent hexadecimal 
void createMap(unordered_map<string, char> *um){ 
    (*um)["0000"] = '0'; 
    (*um)["0001"] = '1'; 
    (*um)["0010"] = '2'; 
    (*um)["0011"] = '3'; 
    (*um)["0100"] = '4'; 
    (*um)["0101"] = '5'; 
    (*um)["0110"] = '6'; 
    (*um)["0111"] = '7'; 
    (*um)["1000"] = '8'; 
    (*um)["1001"] = '9'; 
    (*um)["1010"] = 'A'; 
    (*um)["1011"] = 'B'; 
    (*um)["1100"] = 'C'; 
    (*um)["1101"] = 'D'; 
    (*um)["1110"] = 'E'; 
    (*um)["1111"] = 'F'; 
}
//End of function createMap
 
//Functiom to convert binary string to hexadecimal string 
string bin2Hex(string bin){ 
    int l=bin.size(); 
    int t=bin.find_first_of('.');
    int len_left=t!=-1?t:l;

    for(int i=1;i<=(4-len_left%4)%4;i++) 
        bin='0'+bin;   
    if(t!=-1){
        int len_right=l-len_left-1; 
        for(int i=1;i<=(4-len_right%4)%4;i++) 
            bin=bin+'0';
    }
    unordered_map<string, char> bin_hex_map; 
    createMap(&bin_hex_map); 
      
    int i=0; 
    string hex= "";
    while(1){ 
        hex+=bin_hex_map[bin.substr(i, 4)]; 
        i+=4; 
        if(i==bin.size()) 
        	break;
        if(bin.at(i)=='.'){ 
            hex += '.'; 
            i++; 
        }
    } 
    return hex;     
}
//End of function bin2hex

//Function to extract RS1, RS2 & RD of R type instructions
string otherDataFieldRtype(string &line, string &machineCodeInstructionBinary, string &rs1, string &rs2, string &rd, int i){
	int temp=0;
	while(line[i]!='x' && (i<=line.size()-1)){
		if(line[i]!=' '){
			cout<<"Invalid Register"<<endl;
			return "error";
		}
		i++;
	}
	if(line[i]!='x'){
		cout<<"Invalid Register"<<endl;
		return "error";
	}
	i++;
	if(line[i]!='0' && line[i]!='1' && line[i]!='2' && line[i]!='3' && line[i]!='4' && line[i]!='5' && line[i]!='6' && line[i]!='7' && line[i]!='8' && line[i]!='9'){
		cout<<"Invalid Register"<<endl;
		return "error";
	}
	while(line[i]!=',' && line[i]!=' '){
		if(line[i]!='0' && line[i]!='1' && line[i]!='2' && line[i]!='3' && line[i]!='4' && line[i]!='5' && line[i]!='6' && line[i]!='7' && line[i]!='8' && line[i]!='9'){
			cout<<"Invalid Register"<<endl;
			return "error";
		}
		temp=temp*10+(int)(line[i++]-'0');
	}
	if(temp>31){
		cout<<"Invalid Register"<<endl;
		return "error";
	}
	rd=dec2Binary(temp, 5);

	temp=0;
	while(line[i]!='x' && i<=line.size()-1){
		if(line[i]!=' ' && line[i]!=','){
			cout<<"Invalid Register"<<endl;
			return "error";
		}
		i++;
	}
	if(line[i]!='x'){
		cout<<"Invalid Register"<<endl;
		return "error";
	}
	i++;
	if(line[i]!='0' && line[i]!='1' && line[i]!='2' && line[i]!='3' && line[i]!='4' && line[i]!='5' && line[i]!='6' && line[i]!='7' && line[i]!='8' && line[i]!='9'){
		cout<<"Invalid Register"<<endl;
		return "error";
	}
	while(line[i]!=','&&line[i]!=' '){
		if(line[i]!='0' && line[i]!='1' && line[i]!='2' && line[i]!='3' && line[i]!='4' && line[i]!='5' && line[i]!='6' && line[i]!='7' && line[i]!='8' && line[i]!='9'){
			cout<<"Invalid Register"<<endl;
			return "error";
		}
		temp=temp*10+(int)(line[i++]-'0');
	}
	if(temp>31){
		cout<<"Invalid Register"<<endl;
		return "error";
	}
	rs1=dec2Binary(temp, 5);

	temp=0;
	while(line[i]!='x' && i<=line.size()-1){
		if(line[i]!=' ' && line[i]!=','){
			cout<<"Invalid Register"<<endl;
			return "error";
		}
		i++;
	}
	if(line[i]!='x'){
		cout<<"Invalid Register"<<endl;
		return "error";
	}
	i++;
	if(line[i]!='0' && line[i]!='1' && line[i]!='2' && line[i]!='3' && line[i]!='4' && line[i]!='5' && line[i]!='6' && line[i]!='7' && line[i]!='8' && line[i]!='9'){
		cout<<"Invalid Register"<<endl;
		return "error";
	}

	while (line[i] == '0' || line[i] == '1' || line[i] == '2' || line[i] == '3' || line[i] == '4' || line[i] == '5' || line[i] == '6' || line[i] == '7' || line[i] == '8' || line[i] == '9')
			temp = temp * 10 + (int)(line[i++] - '0');
		
	if(temp>31){
		cout<<"Invalid Register"<<endl;
		return "error";
	}
	rs2=dec2Binary(temp, 5);
	return "valid";
}
//End of function otherDataFieldRtype

//Function to extract rs, rd and immediate values
string otherDataFieldItype(string &line, string &rs1, string &immediate, string &rd, int i, int ISubType){
	int temp = 0;
	int flag1=0;
	// read destination register
	while (i<=line.size()-1 && line[i] != 'x'){
		if(line[i]!=' '){
			cout<<"Invalid Register"<<endl;
			return "error";
		}
		i++;
	}
	if(line[i]!='x'){
		cout<<"Invalid Register"<<endl;
		return "error";
	}
	i++;

	if(line[i]!='0' && line[i]!='1' && line[i]!='2' && line[i]!='3' && line[i]!='4' && line[i]!='5' && line[i]!='6' && line[i]!='7' && line[i]!='8' && line[i]!='9'){
		cout<<"Invalid Register"<<endl;
		return "error";
	}
	while(line[i]!=',' && line[i]!=' '){
		if(line[i]!='0' && line[i]!='1' && line[i]!='2' && line[i]!='3' && line[i]!='4' && line[i]!='5' && line[i]!='6' && line[i]!='7' && line[i]!='8' && line[i]!='9'){
			cout<<"Invalid Register"<<endl;
			return "error";
		}
		temp=temp*10+(int)(line[i++]-'0');
	}
	if(temp>31){
		cout<<"Invalid Register"<<endl;
		return "error";
	}
	rd = dec2Binary(temp, 5);

	temp = 0;
	if(ISubType == 0 || ISubType == 2){ //immediate value

		temp = 0;
		//read source register
		while (i<=line.size()-1 && line[i] != 'x'){
			if(line[i]!=' '&&line[i]!=','){
				cout<<"Invalid Register"<<endl;
				return "error";
			}
			i++;
		}
		i++;
		if(line[i]!='0' && line[i]!='1' && line[i]!='2' && line[i]!='3' && line[i]!='4' && line[i]!='5' && line[i]!='6' && line[i]!='7' && line[i]!='8' && line[i]!='9'){
			cout<<"Invalid Register"<<endl;
			return "error";
		}
		while (line[i]!=',' && line[i]!=' '){
			if(line[i]!='0' && line[i]!='1' && line[i]!='2' && line[i]!='3' && line[i]!='4' && line[i]!='5' && line[i]!='6' && line[i]!='7' && line[i]!='8' && line[i]!='9'){
				cout<<"Invalid Register"<<endl;
				return "error";
			}
			temp = temp * 10 + (int)(line[i++] - '0');
		}
		if(temp>31){
			cout<<"Invalid Register"<<endl;
			return "error";
		}
		rs1 = dec2Binary(temp, 5);

		// skip spaces or commas
		while (i<=line.size()-1 && (line[i]==' '||line[i]==',')){
			i++;
		}
		//for negative offset
		if(line[i]=='-'){
			flag1=1;
			i++;
		}
		//read offset
		if(line[i]!='0' && line[i]!='1' && line[i]!='2' && line[i]!='3' && line[i]!='4' && line[i]!='5' && line[i]!='6' && line[i]!='7' && line[i]!='8' && line[i]!='9'){
			cout<<"Invalid Instruction Format"<<endl;
			return "error";
		}
		temp = 0;
		while (line[i] == '0' || line[i] == '1' || line[i] == '2' || line[i] == '3' || line[i] == '4' || line[i] == '5' || line[i] == '6' || line[i] == '7' || line[i] == '8' || line[i] == '9'){
			temp = temp * 10 + (int)(line[i++] - '0');
		}
		if(flag1==1){
			temp=-1*temp;
		}
		if(ISubType == 0){
			if(temp<-2048||temp>2047){
				cout<<"Out of Range Immediate"<<endl;
				return "error";
			}
			immediate = dec2Binary(temp, 12);
		}
		else{
			if(temp<-16||temp>15){
				cout<<"Out of Range Immediate"<<endl;
				return "error";
			}
			immediate = dec2Binary(temp ,5);
		}
		return "valid";
	}
	else if(ISubType == 1){	//with offset
		temp = 0;
		//skip spaces and commas
		while (line[i]==' '||line[i]==','){
			i++;
		}
		//calculate offset
		if(line[i]!='0' && line[i]!='1' && line[i]!='2' && line[i]!='3' && line[i]!='4' && line[i]!='5' && line[i]!='6' && line[i]!='7' && line[i]!='8' && line[i]!='9'){
			cout<<"Invalid Instruction Format"<<endl;
			return "error";
		}
		while (line[i] == '0' || line[i] == '1' || line[i] == '2' || line[i] == '3' || line[i] == '4' || line[i] == '5' || line[i] == '6' || line[i] == '7' || line[i] == '8' || line[i] == '9')
			temp = temp * 10 + (int)(line[i++] - '0');
		if(temp<-2048||temp>2047){
			cout<<"Out of Range Immediate"<<endl;
			return "error";
		}
		immediate = dec2Binary(temp, 12);

		//read source register
		while(line[i]==' '){
			i++;
		}
		if(line[i]!='('){
			cout<<"Invalid Instruction Format"<<endl;
			return "error";
		}
		i++;
		if(line[i]!='x'){
			cout<<"Invalid Instruction Format"<<endl;
			return "error";
		}
		temp = 0;
		i++;
		if(line[i]!='0' && line[i]!='1' && line[i]!='2' && line[i]!='3' && line[i]!='4' && line[i]!='5' && line[i]!='6' && line[i]!='7' && line[i]!='8' && line[i]!='9'){
			cout<<"Invalid Instruction Format"<<endl;
			return "error";
		}
		while (line[i] == '0' || line[i] == '1' || line[i] == '2' || line[i] == '3' || line[i] == '4' || line[i] == '5' || line[i] == '6' || line[i] == '7' || line[i] == '8' || line[i] == '9')
			temp = temp * 10 + (int)(line[i++] - '0');
		if(temp<-16 && temp>15){
			cout<<"Out of Range Immediate"<<endl;
			return "error";
		}
		rs1 = dec2Binary(temp, 5);
		return "valid";
	}
}
//End of otherDataFieldItype

//Function to extract data fied of S type
string otherDataFieldStype(string &line, string &rs2, string &immediate, string &rs1, int i){
	int temp = 0;
	int flag1=0;
	// skip spaces
	while (line[i]==' '){
		i++;
	}
	if(line[i]!='x'){
		cout<<"Invalid Register"<<endl;
		return "error";
	}
	i++;
	while (line[i] == '0' || line[i] == '1' || line[i] == '2' || line[i] == '3' || line[i] == '4' || line[i] == '5' || line[i] == '6' || line[i] == '7' || line[i] == '8' || line[i] == '9')
		temp = temp * 10 + (int)(line[i++] - '0');
	if(temp>31){
		cout<<"Invalid Register"<<endl;
		return "error";
	}
	rs2 = dec2Binary(temp, 5);
	while(line[i]==' '||line[i]==','){
		i++;
	}
	if(line[i]=='-'){
		flag1=1;
		i++;
	}
	if(line[i]!='0' && line[i]!='1' && line[i]!='2' && line[i]!='3' && line[i]!='4' && line[i]!='5' && line[i]!='6' && line[i]!='7' && line[i]!='8' && line[i]!='9'){
		cout<<"Invalid Instruction Format"<<endl;
		return "error";
	}
	temp=0;
	while(line[i] == '0' || line[i] == '1' || line[i] == '2' || line[i] == '3' || line[i] == '4' || line[i] == '5' || line[i] == '6' || line[i] == '7' || line[i] == '8' || line[i] == '9')
		temp = temp*10 + (int)(line[i++]-'0');
	if(flag1==1){
		temp=-1*temp;
	}
	if(temp>2047||temp<-2048){
		cout<<"Invalid Immediate"<<endl;
		return "error";
	}	
	immediate = dec2Binary(temp,12);
	while(line[i]==' '){
		i++;
	}
	if(line[i]!='('){
		cout<<"Invalid Instruction Format"<<endl;
		return "error";
	}
	i++;
	while(line[i]==' '){
		i++;
	}
	if(line[i]!='x'){
		cout<<"Invalid Register"<<endl;
		return "error";
	}
	i++;
	if(line[i]!='0' && line[i]!='1' && line[i]!='2' && line[i]!='3' && line[i]!='4' && line[i]!='5' && line[i]!='6' && line[i]!='7' && line[i]!='8' && line[i]!='9'){
		cout<<"Invalid Register"<<endl;
		return "error";
	}
	temp=0;
	while(line[i] == '0' || line[i] == '1' || line[i] == '2' || line[i] == '3' || line[i] == '4' || line[i] == '5' || line[i] == '6' || line[i] == '7' || line[i] == '8' || line[i] == '9')
		temp = temp*10 + (int)(line[i++]-'0');
	if(temp>31){
		cout<<"Invalid Register"<<endl;
		return "error";
	}
	rs1 = dec2Binary(temp,5);
	return "valid";
}
//End of otherDataFieldStype

//Function to extract values of SB type
string otherDataFieldSBtype(string &line, string &immediate, string &rs1, string &rs2, lli i, lli currentLineNumber, vector<labelData> &labelArray){
	int temp=0, flag=0;//To check whether such label exist or not
	int flag1=0;
	lli labelOffset;
	string label="";
	while(line[i]==' ')
			i++;
	if(line[i]!='x'){
		cout<<"Invalid Register"<<endl;
		return "error";
	}
	i++;
	if(line[i]!='0' && line[i]!='1' && line[i]!='2' && line[i]!='3' && line[i]!='4' && line[i]!='5' && line[i]!='6' && line[i]!='7' && line[i]!='8' && line[i]!='9'){
		cout<<"Invalid Register"<<endl;
		return "error";
	}
	while(line[i]=='0'||line[i]=='1'||line[i]=='2'||line[i]=='3'||line[i]=='4'||line[i]=='5'||line[i]=='6'||line[i]=='7'||line[i]=='8'||line[i]=='9')
		temp=temp*10+(int)(line[i++]-'0');
	if(temp>31){
		cout<<"Invalid Register"<<endl;
		return "error";
	}
	rs1=dec2Binary(temp, 5);
	while(line[i]==' '||line[i]==',')
		i++;
	if(line[i]!='x'){
		cout<<"Invalid Register"<<endl;
		return "error";
	}
	i++;
	temp=0;
	if(line[i]!='0' && line[i]!='1' && line[i]!='2' && line[i]!='3' && line[i]!='4' && line[i]!='5' && line[i]!='6' && line[i]!='7' && line[i]!='8' && line[i]!='9'){
		cout<<"Invalid Register"<<endl;
		return "error";
	}
	while(line[i]=='0'||line[i]=='1'||line[i]=='2'||line[i]=='3'||line[i]=='4'||line[i]=='5'||line[i]=='6'||line[i]=='7'||line[i]=='8'||line[i]=='9')
		temp=temp*10+(int)(line[i++]-'0');
	if(temp>31){
		cout<<"invalid register"<<endl;
		return "error";
	}
	rs2=dec2Binary(temp, 5);
	temp=0;
	i++;
	while(line[i]==' '||line[i]==',')
		i++;
	while(i<line.length()){
		if(line[i]==' ') 
			break;
		label+=line[i++]; 
	}

	for(i=0;i<labelArray.size();i++)
		if(labelArray[i].label==label){
			labelOffset=(labelArray[i].lineNumber-currentLineNumber)*4;
			if(labelOffset<-1024||labelOffset>2048){
				cout<<"out of range offset"<<endl;
				return "error";
			}
			immediate=dec2Binary(labelOffset, 12);
			flag=1;
		}

	if(flag==0){
		cout<<"ERROR: Incorrect label"<<endl;
		return "error";
	}
	return "valid";
}
//End of otherDataFieldSBtype

string otherDataFieldUJtype(string &line, string &immediate, string &rd, lli i, lli currentLineNumber, vector<labelData> &labelArray){
	int temp=0;
	int flag=0;//To check whether such label exist or not
	lli labelOffset;
	string label="";
	while(line[i]==' ')
		i++;
	if(line[i]!='x'){
		cout<<"Invalid Register"<<endl;
		return "error";
	}
	i++;
	if(line[i]!='0' && line[i]!='1' && line[i]!='2' && line[i]!='3' && line[i]!='4' && line[i]!='5' && line[i]!='6' && line[i]!='7' && line[i]!='8' && line[i]!='9'){
		cout<<"Invalid Register"<<endl;
		return "error";
	}
	while(line[i]=='0'||line[i]=='1'||line[i]=='2'||line[i]=='3'||line[i]=='4'||line[i]=='5'||line[i]=='6'||line[i]=='7'||line[i]=='8'||line[i]=='9')
		temp=temp*10+(int)(line[i++]-'0');
	if(temp>31){
		cout<<"Invalid Register"<<endl;
		return "error";
	}
	rd=dec2Binary(temp, 5);
	while(line[i]==' '||line[i]==',')
		i++;

	while(i<line.length()){
		if(line[i]==' ') 
			break;
		label+=line[i++]; 
	}

	for(i=0;i<labelArray.size();i++)
		if(labelArray[i].label==label){
			labelOffset=(labelArray[i].lineNumber-currentLineNumber)*4;
			if(labelOffset<-262144||labelOffset>262143){
				cout<<"out of range label"<<endl;
				return "error";
			}
			immediate=dec2Binary(labelOffset, 20);
			flag=1;
		}
	if(flag==0){
		cout<<"ERROR: Incorrect label"<<endl;
		return "error";
	}
	return "valid";
}

string otherDataFieldUtype(string &line,string &immediate,string &rd,lli i){
	int temp=0;
	while(line[i]==' ')
		i++;
	if(line[i]!='x'){
		cout<<"Invalid Register"<<endl;
		return "error";
	}
	i++;
	if(line[i]!='0' && line[i]!='1' && line[i]!='2' && line[i]!='3' && line[i]!='4' && line[i]!='5' && line[i]!='6' && line[i]!='7' && line[i]!='8' && line[i]!='9'){
		cout<<"Invalid Register"<<endl;
		return "error";
	}
	while(line[i]=='0'||line[i]=='1'||line[i]=='2'||line[i]=='3'||line[i]=='4'||line[i]=='5'||line[i]=='6'||line[i]=='7'||line[i]=='8'||line[i]=='9')
		temp=temp*10+(int)(line[i++]-'0');
	if(temp>31){
		cout<<"Invalid Register"<<endl;
		return "error";
	}
	rd=dec2Binary(temp, 5);
	while(line[i]==' '||line[i]==',')
		i++;
	if(line[i]!='0' && line[i]!='1' && line[i]!='2' && line[i]!='3' && line[i]!='4' && line[i]!='5' && line[i]!='6' && line[i]!='7' && line[i]!='8' && line[i]!='9'){
		cout<<"Invalid Immediate"<<endl;
		return "error";
	}
	temp=0;
	while(line[i]=='0'|| line[i] == '1' || line[i] == '2' || line[i] == '3' || line[i] == '4' || line[i] == '5' || line[i] == '6' || line[i] == '7' || line[i] == '8' || line[i] == '9')
		temp = temp * 10 + (int)(line[i++] - '0');
	if(temp<-524288||temp>524287){
		cout<<"Invalid Immediate"<<endl;
		return "error";
	}
	immediate=dec2Binary(temp,20);
	return "valid";
}

/*Assembly to Machine Code
Input: each line of assembly code, line number, label array
Output: equivalent machine code(hexadecimal string)*/
string asm2mc(string line, lli currentLineNumber, vector<labelData> &labelArray){
	string instruction="";//first word of each assembly line
	string machineCodeInstructionBinary="";
	string machineCodeInstructionHex="";
	string opcode="",funct3="",funct7="";
	string immediate="";
	string rs1="", rs2="", rd=""; 
	string type="";
	int ISubType = 0;//subtypes for I-type instructions 0 for addi etc, 1 for instruction with offset, 2 for shift instructions

	int i=0;
	while(i<line.size()&&line[i]!=' '){
		if(line[i]==':'){
			instruction+=line[i];
			break;
		}
		instruction+=line[i];
		i++;
	}
	if(instruction[instruction.size()-1]==':'){
		type="LABEL";
		if(i<=line.size()-2){
			instruction="";
			i++;
			while(line[i]==' ')
				i++;
			while(line[i]!=' ')
				instruction+=line[i++];
		}
	}
	if(instruction=="add")
		opcode="0110011", funct3="000", funct7="0000000", type="R";

	if(instruction=="and")
		opcode="0110011", funct3="111", funct7="0000000", type="R";
		
	if(instruction=="or")
		opcode="0110011", funct3="110", funct7="0000000", type ="R";

	if(instruction=="sll")
		opcode="0110011", funct3="001", funct7="0000000", type ="R";
		
	if(instruction=="slt")
		opcode="0110011", funct3="010", funct7="0000000", type ="R";

	if(instruction=="sltu")
		opcode="0110011", funct3="011", funct7="0000000", type ="R";

	if(instruction=="sra")
		opcode="0110011", funct3="101", funct7="0100000", type ="R";
	
	if(instruction=="srl")
		opcode="0110011", funct3="101", funct7="0000000", type ="R";
	
	if(instruction=="sub")
		opcode="0110011", funct3="000", funct7="0100000", type ="R";
	
	if(instruction=="xor")
		opcode="0110011", funct3="100", funct7="0000000", type ="R";
	
	if(instruction=="mul")
		opcode="0110011", funct3="000", funct7="0000001", type ="R";
	
	if(instruction=="div")
		opcode="0110011", funct3="100", funct7="0000001", type ="R";
		
	if(instruction=="divu")
		opcode="0110011", funct3="101", funct7="0000001", type ="R";
		
	if(instruction=="rem")
		opcode="0110011", funct3="110", funct7="0000001", type ="R";
		
	if(instruction=="remu")
		opcode="0110011", funct3="111", funct7="0000001", type ="R";
		
	if(instruction=="addw")
		opcode="0111011", funct3="000", funct7="0000000", type  ="R";
		
	if(instruction=="subw")
		opcode="0111011", funct3="000", funct7="0100000", type ="R";
		
	if(instruction=="sllw")
		opcode="0111011", funct3="001", funct7="0000000", type ="R";
		
	if(instruction=="srlw")
		opcode="0111011", funct3="101", funct7="0000000", type ="R";
		
	if(instruction=="sraw")
		opcode="0111011", funct3="101", funct7="0100000", type ="R";
	
	if(instruction == "lb")
		opcode = "0000011", funct3 = "000", type = "I", ISubType = 1;

	if(instruction == "lh")
		opcode = "0000011", funct3 = "001", type = "I", ISubType = 1;

	if(instruction == "lw")
		opcode = "0000011", funct3 = "010", type = "I", ISubType = 1;
	
	if(instruction == "ld")
		opcode = "0000011", funct3 = "011", type = "I", ISubType = 1;

	if(instruction == "lbu")
		opcode = "0000011", funct3 = "100", type = "I", ISubType = 1;

	if(instruction == "lhu")
		opcode = "0000011", funct3 = "101", type = "I", ISubType = 1;

	if(instruction == "lwu")
		opcode = "0000011", funct3 = "110", type = "I", ISubType = 1;

	if(instruction == "addi")
		opcode = "0010011", funct3 = "000", type = "I";
	
	if(instruction == "slli")
		opcode = "0010011", funct3 = "001",funct7 = "0000000", type = "I", ISubType = 2;

	if(instruction == "srli")
		opcode = "0010011", funct3 = "101", funct7 = "0000000", type = "I", ISubType = 2;

	if(instruction == "srai")
		opcode = "0010011", funct3 = "001", funct7 = "0100000", type = "I", ISubType = 2;

	if(instruction == "slliw")
		opcode = "0011011", funct3 = "001", funct7 = "0000000", type = "I", ISubType = 2;

	if(instruction == "srliw")
		opcode = "0011011", funct3 = "101", funct7 = "0000000", type = "I", ISubType = 2;

	if(instruction == "sraiw")
		opcode = "0011011", funct3 = "101", funct7 = "0100000", type = "I", ISubType = 2;

	if(instruction == "slti")
		opcode = "0010011", funct3 = "010", type = "I";

	if(instruction == "sltiu")
		opcode = "0010011", funct3 = "011", type = "I";

	if(instruction == "xori")
		opcode = "0010011", funct3 = "100", type = "I";

	if(instruction == "slti")
		opcode = "0010011", funct3 = "010", type = "I";

	if(instruction == "ori")
		opcode = "0010011", funct3 = "110", type = "I";
	
	if (instruction == "andi")
		opcode = "0010011", funct3 = "111", type = "I";

	if (instruction == "addiw")
		opcode = "0010011", funct3 = "000", type = "I";

	if (instruction == "sd")
		opcode = "0010011", funct3 = "011", type = "I";

	if (instruction == "jalr")
		opcode = "1100111", funct3 = "000", type = "I", ISubType = 1;
	
	if(instruction == "sw")
		opcode = "0100011", funct3 = "010", type = "S";

	if(instruction == "sh")
		opcode = "0100011", funct3 = "001", type = "S";

	if(instruction == "sb")
		opcode = "0100011", funct3 = "000", type = "S";

	if(instruction=="beq")
		opcode="1100011",funct3="000",type="SB";

	if(instruction=="bne")
		opcode="1100011",funct3="001",type="SB";

	if(instruction=="blt")
		opcode="1100011",funct3="100",type="SB";

	if(instruction=="bge")
		opcode="1100011",funct3="101",type="SB";

	if(instruction=="bltu")
		opcode="1100011",funct3="110",type="SB";

	if(instruction=="bgeu")
		opcode="1100011",funct3="111",type="SB";
	
	if(instruction=="jal")
		opcode="1101111",type="UJ";
	
	if(instruction=="auipc")
		opcode="0010111",type="U";

	if(instruction=="lui")
		opcode="0110111", type="U";

	//To extract other data field according to instruction type
	if(type=="R"){
		string s = otherDataFieldRtype(line, machineCodeInstructionBinary, rs1, rs2, rd, i);
		if(s!="error")
			machineCodeInstructionBinary=funct7+rs2+rs1+funct3+rd+opcode;
	}
	
	else if(type == "I"){
		string s = otherDataFieldItype(line, rs1, immediate, rd, i, ISubType);
		if(s=="error")
			machineCodeInstructionBinary="";
		else if(ISubType == 0 || ISubType == 1){
			machineCodeInstructionBinary = immediate + rs1 + funct3 + rd + opcode;
		}
		else if(ISubType == 2){
			machineCodeInstructionBinary = funct7 + immediate + rs1 + funct3 + rd + opcode;
		}
	}
	
	else if(type=="S"){
		string s = otherDataFieldStype(line,rs2,immediate,rs1,i);
		//cout<<"s = "<<s<<endl;
		string imm_4_0 = immediate.substr(7,5);
		string imm_11_5 = immediate.substr(0,7);
		if(s!="error")
			machineCodeInstructionBinary = imm_11_5+rs2+rs1+funct3+imm_4_0+opcode;
	}
	
	else if(type=="SB"){
		string s = otherDataFieldSBtype(line, immediate, rs1, rs2, i, currentLineNumber, labelArray);
		if(s!="error")
			machineCodeInstructionBinary=immediate[0]+immediate.substr(1,6)+rs2+rs1+funct3+immediate.substr(7,4)+immediate[0]+opcode;
	}
	else if(type=="UJ"){
		string s = otherDataFieldUJtype(line,immediate,rd,i,currentLineNumber,labelArray);
		if(s!="error")
			machineCodeInstructionBinary = immediate[0]+immediate.substr(9,10)+immediate[8]+immediate.substr(0,8)+rd+opcode;
	}
	else if(type=="U"){
		string s = otherDataFieldUtype(line,immediate,rd,i);
		if(s!="error")
			machineCodeInstructionBinary = immediate+rd+opcode;
	}
	else if(type=="LABEL")
		return "labelDetected";
	else {
		cout<<"Unsupported Instruction"<<endl;
	}
	if(machineCodeInstructionBinary!="")
		machineCodeInstructionHex="0x"+bin2Hex(machineCodeInstructionBinary);
	else
		machineCodeInstructionHex="";
	return machineCodeInstructionHex;
}
//End of function asm2mc

/*Keep track of labels in a vector
labelArray stores label & their line number */
void assignLineNumberToLabel(vector<labelData> &labelArray){
	lli lineNumber=0, flag=0, flag1=0;
	labelData tempLabel;
	string line="";

	fstream fileReading;
	fileReading.open("assemblyCode1.asm");
	int datasegment=0;
	while(getline(fileReading,line)){
		flag=0;
		int i=0;
		string label="";
		if(line==".data")
			datasegment=1;
		if(line==".text")
			datasegment=0;
		if(datasegment==0 && line!=".text"){
		if(line.size()==0 )
			continue;
		while(line[i]==' ')
			i++;
		while(i<line.size()){
			if(line[i]==':'){
				flag=1;
				break;
			}
			label+=line[i++];
		}
		if(flag==1){
			if(i<line.size()){
				i++;
			while(line[i]==' ')
				i++;
			if(i<=line.size()-2)
				flag1=1;
			}
		}
		
		if(flag==1){
			tempLabel.label=label;
			tempLabel.lineNumber=lineNumber;
			labelArray.push_back(tempLabel);
		}
		if(flag==1 && flag1==0)
			continue;

		lineNumber++;
	}
	}
	fileReading.close();
}
//End of assignLineNumberToLabel
void removeComments(string str){
	fstream fileReading;
	fileReading.open(str,ios::in);
	fstream fileWriting;
	fileWriting.open("assemblyCode1.asm",ios::out);
	string line="";
	string nline="";
	int i=0;
	while(getline(fileReading,line)){
		i=0;
		nline="";
		if(line[i]=='#')
			nline="";
		else{
			while(i<line.size()){
				if(line[i]=='#')
					break;
				nline+=line[i++];
			}
		}
		fileWriting<<nline<<endl;
	}
}
//Main File : File Read & Write
int main(){
	lli instructionAddress=0, currentLineNumber=0;
	
	string hexInstructionAddress;
	string binaryInstructionAddress;
	string assemblyLine;
	string machineLine="";
	string input;
	vector<data> varArray;
	vector<labelData> labelArray;
	cout<<"Enter the input file name"<<endl;
	cin>>input;
	removeComments(input);
	assignLineNumberToLabel(labelArray);
	int datasegment=0;//to check if .data is there or not
	int textsegment=0;
	long int* memory = new long int[1<<22];
	int memoryused=0;
	
	fstream fileReading;
	fstream fileWriting;
	fstream fileWriting2;
	fileReading.open("assemblyCode1.asm", ios::in);
	fileWriting.open("machineCode.mc", ios::out);
	fileWriting2.open("machineData.txt");
	int datal=1048576;//0x100000
	
	//To read input from Assembly Code File 
	while(getline(fileReading, assemblyLine)){
		cout<<assemblyLine<<endl;
		if(assemblyLine==".data")
			datasegment=1;
		if(assemblyLine==".text"){
			datasegment=0;
			textsegment=1;
		}
		int i = 0;
		if(assemblyLine[0]==' '){
			while(assemblyLine[i]==' ')
				i++;
			assemblyLine = assemblyLine.substr(i,assemblyLine.size()-i);
		}

		if(assemblyLine.size() == 0){
			continue;
		}
		i=0;
		string var="";
		string size="";
		string value="";
		string address="";
		if(datasegment==1 && assemblyLine!=".data"){
			while(assemblyLine[i]!=':' && i<assemblyLine.size()){
				var+=assemblyLine[i];
				i++;
			}
			if(assemblyLine[i]!=':'){
				cout<<"ERROR"<<endl;
				goto EXIT;
			}
			i++;
			while(assemblyLine[i]==' ')
				i++;
			i++;
			while(assemblyLine[i]!=' '){
				size+=assemblyLine[i];
				i++;
			}
			i++;
			while(i<assemblyLine.size()){
				value="";
				while(assemblyLine[i]=='0'||assemblyLine[i]=='1'||assemblyLine[i]=='2'||assemblyLine[i]=='3'||assemblyLine[i]=='4'||assemblyLine[i]=='5'||assemblyLine[i]=='6'||assemblyLine[i]=='7'||assemblyLine[i]=='8'||assemblyLine[i]=='9'){
					value+=assemblyLine[i++];
				}
				address = bitset<24>(datal).to_string();
				address = bin2Hex(address);
				machineLine = value+" 0x"+address+" "+size;
				if(size=="byte")
					datal++;
				else if(size=="word")
					datal+=4;
				data obj;
				obj.var=var;
				obj.hexaddress="0x"+address;
				varArray.push_back(obj);
				i++;
				fileWriting2<<machineLine<<endl;
			}
		}
		else if(assemblyLine!=".text" && assemblyLine!=".data"){
			machineLine=asm2mc(assemblyLine, currentLineNumber, labelArray);
		if(machineLine!="labelDetected"){
			if(machineLine!=""){
				binaryInstructionAddress=dec2Binary(instructionAddress, 32);
				instructionAddress+=4;
				hexInstructionAddress="0x"+bin2Hex(binaryInstructionAddress);
				machineLine=hexInstructionAddress+" "+machineLine;
				fileWriting<<machineLine<<endl;
				currentLineNumber++;
			}
		}
	}
}
	EXIT:
	fileWriting.close();
	fileWriting2.close();
	fileReading.close();
}
//End of main
//instructions to be used for load address
// auipc rd, symbol[31:12] + 1
// addi rd, rd, symbol[11:0]
//https://github.com/riscv/riscv-isa-manual/issues/144
