#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>
#include<queue>
using namespace std;
#define MemSize 1000 // memory size, in reality, the memory size should be 2^32, but for this lab, for the space resaon, we keep it as this large numberdr, but the memory is still 32-bit addressable.
struct IFStruct {
    bitset<32>  PC = 0;
    bool        nop = false;  
};
struct IDStruct {
	bitset<32>  PC = 0;
    bitset<32>  Instr;
    bool        nop = true;  
};
struct EXStruct {
	bitset<32>  PC = 0;
    bitset<32>  Read_data1;// data storage for rs1
    bitset<32>  Read_data2;// data storage for rs2
    bitset<16>  Imm;
    bitset<5>   Rs;//rs1
    bitset<5>   Rt;//rs2
    bitset<5>   Wrt_reg_addr;//rd
    bool        rd_mem;// read data memory
    bool        wrt_mem; // write to dmem
    bool        alu_op;     //1 for addu, lw, sw, 0 for subu 
    bool        wrt_enable;// write to reg
    bool        nop = true;
	bool        rs1 = false;// need to read register
	bool        rs2 = false;// need to read register
	string  INS;  
};
struct MEMStruct {
    bitset<32>  ALUresult;
    bitset<32>  Store_data;
    bitset<5>   Rs;//rs1
    bitset<5>   Rt;//rs2   
    bitset<5>   Wrt_reg_addr;//rd
    bool        rd_mem = 0;// read data memory
    bool        wrt_mem = 0;  // write to dmem
    bool        wrt_enable = 0; // write to reg   
    bool        nop = true;
	string  INS;    
};
struct WBStruct {
    bitset<32>  Wrt_data;
    bitset<5>   Rs;//rs1
    bitset<5>   Rt;//rs2    
    bitset<5>   Wrt_reg_addr;//rd
    bool        wrt_enable = 0;//write to reg
    bool        nop = true;   
};
struct stateStruct {
    IFStruct    IF;
    IDStruct    ID;
    EXStruct    EX;
    MEMStruct   MEM;
    WBStruct    WB;
};

class InsMem
{
	public:
		string id, ioDir;
        InsMem(string name, string ioDir) {       
			id = name;
			IMem.resize(MemSize);
            ifstream imem;
			string line;
			int i=0;
			imem.open(ioDir + "\\imem.txt");
			if (imem.is_open())
			{
				while (getline(imem,line))
				{      
					IMem[i] = bitset<8>(line);
					i++;
				}                    
			}
            else cout<<"Unable to open IMEM input file.";
			imem.close();                     
		}

		bitset<32> readInstr(bitset<32> ReadAddress) {    
			string instruction;
            instruction.append(IMem[ReadAddress.to_ulong()].to_string());
            instruction.append(IMem[ReadAddress.to_ulong()+1].to_string());
            instruction.append(IMem[ReadAddress.to_ulong()+2].to_string());
            instruction.append(IMem[ReadAddress.to_ulong()+3].to_string());
            return bitset<32>(instruction); 
		}     
      
    private:
        vector<bitset<8> > IMem;     
};
      
class DataMem    
{
    public: 
		string id, opFilePath, ioDir;
        DataMem(string name, string ioDir) : id{name}, ioDir{ioDir} {
            DMem.resize(MemSize);
			opFilePath = ioDir + "\\" + name + "_DMEMResult.txt";
            ifstream dmem;
            string line;
            int i=0;
            dmem.open(ioDir + "\\dmem.txt");
            if (dmem.is_open()) {
                while (getline(dmem,line))
                {      
                    DMem[i] = bitset<8>(line);
                    i++;
                }
            }
            else cout<<"Unable to open DMEM input file.";
            dmem.close();          
        }
		
        bitset<32> readDataMem(bitset<32> Address) {
            //cout<<"\tReading memory "<<Address<<endl;	
			string datamem;
            datamem.append(DMem[Address.to_ulong()].to_string());
            datamem.append(DMem[Address.to_ulong()+1].to_string());
            datamem.append(DMem[Address.to_ulong()+2].to_string());
            datamem.append(DMem[Address.to_ulong()+3].to_string());
            return bitset<32>(datamem); 
		}
            
        void writeDataMem(bitset<32> Address, bitset<32> WriteData) {
            cout<<"\tWriting "<< WriteData <<" to memory "<<Address<<endl;	
            DMem[Address.to_ulong()] = bitset<8>(WriteData.to_string().substr(0, 8));
            DMem[Address.to_ulong() + 1] = bitset<8>(WriteData.to_string().substr(8, 8));
            DMem[Address.to_ulong() + 2] = bitset<8>(WriteData.to_string().substr(16, 8));
            DMem[Address.to_ulong() + 3] = bitset<8>(WriteData.to_string().substr(24, 8));
			//cout<<DMem[Address.to_ulong()]<<endl;
			//cout<<DMem[Address.to_ulong()+1]<<endl;
			//cout<<DMem[Address.to_ulong()+2]<<endl;
			//cout<<DMem[Address.to_ulong()+3]<<endl;
        }   
                     
        void outputDataMem() {
            ofstream dmemout;
            dmemout.open(opFilePath, std::ios_base::trunc);
            if (dmemout.is_open()) {
                cout<<"open successful for "<<id<<" DMEM result file." << endl;
				//DMem[8] = bitset<8> (255);
				//cout<<DMem[8]<<endl;
                for (int j = 0; j< 1000; j++)
                {     
                    dmemout << DMem[j]<<endl;
                }
                     
            } else cout<<"Unable to open "<<id<<" DMEM result file." << endl;
            dmemout.close();
        }             

    private:
		vector<bitset<8> > DMem;      
};

class RegisterFile
{
    public:
		string outputFile;
     	RegisterFile(string ioDir): outputFile {ioDir + "RFResult.txt"} {
			Registers.resize(32);  
			Registers[0] = bitset<32> (0);  
        }
        bitset<32> readRF(bitset<5> Reg_addr) {
            return Registers[Reg_addr.to_ulong()];
        }
        void writeRF(bitset<5> Reg_addr, bitset<32> Wrt_reg_data) {
            if (Reg_addr.to_ulong() != 0)Registers[Reg_addr.to_ulong()] = Wrt_reg_data;
        }
		void outputRF(int cycle) {
			ofstream rfout;
			if (cycle == 0)
				rfout.open(outputFile, std::ios_base::trunc);
			else 
				rfout.open(outputFile, std::ios_base::app);
			if (rfout.is_open())
			{
				rfout<<"State of RF after executing cycle:\t"<<cycle<<endl;
				for (int j = 0; j<32; j++)
				{
					rfout << Registers[j]<<endl;
				}
			}
			else cout<<"Unable to open RF output file."<<endl;
			rfout.close();               
		} 
			
	private:
		vector<bitset<32>>Registers;
};
bitset<16> extend(string s) {
	char sign = s.at(0);
	string temp = s;
	while(temp.length() != 16) {
		temp = sign + temp;
		//cout<<temp.length()<<endl;
	}
	return bitset<16>(temp);
}
long to_long(bitset<16> imm) {
	long res = 0;
	unsigned long value = bitset<15>(imm.to_string().substr(1,15)).to_ulong();
	//cout<<(1<<15)<<endl;
	string sign = imm.to_string().substr(0,1);
	res+= value;
	if (sign == "1") {
		return res - (1<<15);
	} else return res;

}
class Core {
	public:
		RegisterFile myRF;
		uint32_t cycle = 0;
		bool halted = false;
		string ioDir;
		struct stateStruct state, nextState;
		InsMem ext_imem;
		DataMem ext_dmem;
		
		Core(string ioDir, InsMem &imem, DataMem &dmem): myRF(ioDir), ioDir{ioDir}, ext_imem {imem}, ext_dmem {dmem} {}

		virtual void step() {}

		virtual void printState() {}
};

class SingleStageCore : public Core {
	public:
		SingleStageCore(string ioDir, InsMem &imem, DataMem &dmem): Core(ioDir + "\\SS_", imem, dmem), opFilePath(ioDir + "\\StateResult_SS.txt") {}

		void step() {
			/* Your implementation*/
            //print some info
            cout<<"Cycle: "<< cycle <<endl;
            cout<<"PC:\t"<<state.IF.PC.to_ulong()<<endl;  
            cout<<"Halt?: "<< (halted? "true":"false") <<endl;
            //Instruction
            bitset<32> Instruction= ext_imem.readInstr(state.IF.PC);         
            cout<<"\tInstruction read:"<<Instruction<<endl;
            string opcode = Instruction.to_string().substr(25,7);
			cout<<"\tThe opcode is " << opcode << endl;
            //detect halt
            if(opcode == "1111111" || opcode == "0000000") {
                cout<<"\tHALTING..."<<endl;
               	nextState.IF.nop = true; 
            } else {
               	nextState.IF.PC = state.IF.PC.to_ulong() + 4; 
               	nextState.IF.nop = false; 
            }
            //read instruction and execute
            if (opcode == "0000011") {// LW
                state.EX.Imm = extend(Instruction.to_string().substr(0,12));
                state.EX.Rs = bitset<5> (Instruction.to_string().substr(12,5));
                state.EX.Wrt_reg_addr = bitset<5> (Instruction.to_string().substr(20,5));
                cout<<"\tThe instruction is lw R" << state.EX.Wrt_reg_addr.to_ulong()<< ", R"<< state.EX.Rs.to_ulong()<< ", #"<< state.EX.Imm<< endl;
                myRF.writeRF(state.EX.Wrt_reg_addr, ext_dmem.readDataMem(myRF.readRF(state.EX.Rs).to_ulong() + to_long(state.EX.Imm)));
                cout<<"\tData writen in register "<< myRF.readRF(state.EX.Wrt_reg_addr)<<endl;
			} else if (opcode == "0100011") {// SW
				cout<<"\tfunction SW..." << endl;
                state.EX.Imm = extend(Instruction.to_string().substr(0,7).append(Instruction.to_string().substr(20,5)));
                state.EX.Rs = bitset<5> (Instruction.to_string().substr(12,5));
                state.EX.Rt = bitset<5> (Instruction.to_string().substr(7,5));
                cout<<"\tThe instruction is sw R" << state.EX.Rt.to_ulong()<< ", R"<< state.EX.Rs.to_ulong()<< ", #"<< state.EX.Imm<< endl;
                ext_dmem.writeDataMem(bitset<32>(myRF.readRF(state.EX.Rs).to_ulong() + state.EX.Imm.to_ulong()), myRF.readRF(state.EX.Rt));
				cout<<"\tData writen in memory "<< ext_dmem.readDataMem(bitset<32>(myRF.readRF(state.EX.Rs).to_ulong() + to_long(state.EX.Imm)))<<endl;
			} else if (opcode == "1100011") {// BNE or BEQ
				string func3 = Instruction.to_string().substr(17,3);
				cout<<"\tThe func3 is " << func3 << endl;
				state.EX.Rs = bitset<5> (Instruction.to_string().substr(12,5));
				state.EX.Rt = bitset<5> (Instruction.to_string().substr(7,5));
				cout<<"\tRs: "<<myRF.readRF(state.EX.Rs).to_ulong()<<endl;
				cout<<"\tRt: "<<myRF.readRF(state.EX.Rt).to_ulong()<<endl;
				string imm;
				imm.append(Instruction.to_string().substr(0,1));
				imm.append(Instruction.to_string().substr(24,1));
				imm.append(Instruction.to_string().substr(1,6));
				imm.append(Instruction.to_string().substr(20,4));
				imm.append("0");
				cout<<"\tIMM: "<<imm<<endl;
				state.EX.Imm = extend(imm);
				if (func3 == "000") {// BEQ
					if (myRF.readRF(state.EX.Rs).to_ulong() == myRF.readRF(state.EX.Rt).to_ulong()) {
						cout<<"\tCurrent PC "<<state.IF.PC.to_ulong()<<endl;
						cout<<"\tCurrent Imm "<<to_long(state.EX.Imm)<<endl;
						nextState.IF.PC = bitset<32>(state.IF.PC.to_ulong() + to_long(state.EX.Imm));
						cout<<"\tBranching taken to "<<nextState.IF.PC.to_ulong()<<endl;
					} else cout<<"\tNo Branching taken "<<endl;
				} else if (func3 == "001") {//BNE
					if (myRF.readRF(state.EX.Rs).to_ulong() != myRF.readRF(state.EX.Rt).to_ulong()) {
						cout<<"\tCurrent PC "<<state.IF.PC.to_ulong()<<endl;
						cout<<"\tCurrent Imm "<<to_long(state.EX.Imm)<<endl;
						nextState.IF.PC = bitset<32>(state.IF.PC.to_ulong() + to_long(state.EX.Imm));
						cout<<"\tBranching taken to "<<nextState.IF.PC.to_ulong()<<endl;
					} else cout<<"\tNo Branching taken "<<endl;
				}
			} else if (opcode == "0110011") {// ADD or SUB or XOR or OR or AND
				string func7 = Instruction.to_string().substr(0,7);
				cout<<"\tThe func7 is " << func7 << endl;
				if (func7 == "0000000") {
					string func3 = Instruction.to_string().substr(17,3);
					cout<<"\tThe func3 is " << func3 << endl;
					if (func3 == "000") {
                        state.EX.Rs = bitset<5> (Instruction.to_string().substr(12,5));
                        state.EX.Rt = bitset<5> (Instruction.to_string().substr(7,5));
                        state.EX.Wrt_reg_addr = bitset<5> (Instruction.to_string().substr(20,5));
                        cout<<"\tThe instruction is add R" << state.EX.Wrt_reg_addr.to_ulong()<< ", R"<< state.EX.Rs.to_ulong()<< ", R"<< state.EX.Rt.to_ulong()<< endl;
                        myRF.writeRF(state.EX.Wrt_reg_addr, bitset<32> (myRF.readRF(state.EX.Rs).to_ulong() + myRF.readRF(state.EX.Rt).to_ulong()));
						cout<<"\tRs1 is \t\t\t"<< myRF.readRF(state.EX.Rs)<<endl;
						cout<<"\tRs2 is \t\t\t"<< myRF.readRF(state.EX.Rt)<<endl;
                        cout<<"\tData writen in register "<< myRF.readRF(state.EX.Wrt_reg_addr)<<endl;
					} else if (func3 == "100") {
						state.EX.Rs = bitset<5> (Instruction.to_string().substr(12,5));
						state.EX.Rt = bitset<5> (Instruction.to_string().substr(7,5));
						state.EX.Wrt_reg_addr = bitset<5> (Instruction.to_string().substr(20,5));
						cout<<"\tThe instruction is xor R" << state.EX.Wrt_reg_addr.to_ulong()<< ", R"<< state.EX.Rs.to_ulong()<< ", R"<< state.EX.Rt.to_ulong()<< endl;
						myRF.writeRF(state.EX.Wrt_reg_addr, bitset<32> (myRF.readRF(state.EX.Rs).to_ulong() ^ myRF.readRF(state.EX.Rt).to_ulong()));
						cout<<"\tRs1 is \t\t\t"<< myRF.readRF(state.EX.Rs)<<endl;
						cout<<"\tRs2 is \t\t\t"<< myRF.readRF(state.EX.Rt)<<endl;
						cout<<"\tData writen in register "<< myRF.readRF(state.EX.Wrt_reg_addr)<<endl;
					} else if (func3 == "110") {
						state.EX.Rs = bitset<5> (Instruction.to_string().substr(12,5));
						state.EX.Rt = bitset<5> (Instruction.to_string().substr(7,5));
						state.EX.Wrt_reg_addr = bitset<5> (Instruction.to_string().substr(20,5));
						cout<<"\tThe instruction is or R" << state.EX.Wrt_reg_addr.to_ulong()<< ", R"<< state.EX.Rs.to_ulong()<< ", R"<< state.EX.Rt.to_ulong()<< endl;
						myRF.writeRF(state.EX.Wrt_reg_addr, bitset<32> (myRF.readRF(state.EX.Rs).to_ulong() | myRF.readRF(state.EX.Rt).to_ulong()));
						cout<<"\tRs1 is \t\t\t"<< myRF.readRF(state.EX.Rs)<<endl;
						cout<<"\tRs2 is \t\t\t"<< myRF.readRF(state.EX.Rt)<<endl;
						cout<<"\tData writen in register "<< myRF.readRF(state.EX.Wrt_reg_addr)<<endl;
					} else if (func3 == "111") {
						state.EX.Rs = bitset<5> (Instruction.to_string().substr(12,5));
						state.EX.Rt = bitset<5> (Instruction.to_string().substr(7,5));
						state.EX.Wrt_reg_addr = bitset<5> (Instruction.to_string().substr(20,5));
						cout<<"\tThe instruction is and R" << state.EX.Wrt_reg_addr.to_ulong()<< ", R"<< state.EX.Rs.to_ulong()<< ", R"<< state.EX.Rt.to_ulong()<< endl;
						myRF.writeRF(state.EX.Wrt_reg_addr, bitset<32> (myRF.readRF(state.EX.Rs).to_ulong() & myRF.readRF(state.EX.Rt).to_ulong()));
						cout<<"\tRs1 is \t\t\t"<< myRF.readRF(state.EX.Rs)<<endl;
						cout<<"\tRs2 is \t\t\t"<< myRF.readRF(state.EX.Rt)<<endl;
						cout<<"\tData writen in register "<< myRF.readRF(state.EX.Wrt_reg_addr)<<endl;
					}
				} else if (func7 == "0100000") {
					state.EX.Rs = bitset<5> (Instruction.to_string().substr(12,5));
                    state.EX.Rt = bitset<5> (Instruction.to_string().substr(7,5));
                    state.EX.Wrt_reg_addr = bitset<5> (Instruction.to_string().substr(20,5));
                    cout<<"\tThe instruction is sub R" << state.EX.Wrt_reg_addr.to_ulong()<< ", R"<< state.EX.Rs.to_ulong()<< ", R"<< state.EX.Rt.to_ulong()<< endl;
                    myRF.writeRF(state.EX.Wrt_reg_addr, bitset<32> (myRF.readRF(state.EX.Rs).to_ulong() - myRF.readRF(state.EX.Rt).to_ulong()));
					cout<<"\tRs1 is \t\t\t"<< myRF.readRF(state.EX.Rs)<<endl;
					cout<<"\tRs2 is \t\t\t"<< myRF.readRF(state.EX.Rt)<<endl;
                    cout<<"\tData writen in register "<< myRF.readRF(state.EX.Wrt_reg_addr)<<endl;
				}
			} else if (opcode == "0010011") {// ADDI or XORI or ORI or ANDI
					string func3 = Instruction.to_string().substr(17,3);
					cout<<"\tThe func3 is " << func3 << endl;
					state.EX.Imm = extend(Instruction.to_string().substr(0,12));
					state.EX.Rs = bitset<5> (Instruction.to_string().substr(12,5));
                    state.EX.Wrt_reg_addr = bitset<5> (Instruction.to_string().substr(20,5));
					if (func3 == "000") {
						cout<<"\tfunction ADDI..." << endl;
						myRF.writeRF(state.EX.Wrt_reg_addr, myRF.readRF(state.EX.Rs).to_ulong() + to_long(state.EX.Imm));
						cout<<"\tThe instruction is addi R" << state.EX.Wrt_reg_addr.to_ulong()<< ", R"<< state.EX.Rs.to_ulong()<< ", #"<< state.EX.Imm<< endl;
						cout<<"\tData writen in register "<< myRF.readRF(state.EX.Wrt_reg_addr)<<endl;
					} else if (func3 == "100") {
						cout<<"\tfunction XORI..." << endl;
						myRF.writeRF(state.EX.Wrt_reg_addr, myRF.readRF(state.EX.Rs).to_ulong() ^ to_long(state.EX.Imm));
						cout<<"\tThe instruction is xori R" << state.EX.Wrt_reg_addr.to_ulong()<< ", R"<< state.EX.Rs.to_ulong()<< ", #"<< state.EX.Imm<< endl;
						cout<<"\tData writen in register "<< myRF.readRF(state.EX.Wrt_reg_addr)<<endl;
					} else if (func3 == "110") {
						cout<<"\tfunction ORI..." << endl;
						myRF.writeRF(state.EX.Wrt_reg_addr, myRF.readRF(state.EX.Rs).to_ulong() | to_long(state.EX.Imm));
						cout<<"\tThe instruction is ori R" << state.EX.Wrt_reg_addr.to_ulong()<< ", R"<< state.EX.Rs.to_ulong()<< ", #"<< state.EX.Imm<< endl;
						cout<<"\tData writen in register "<< myRF.readRF(state.EX.Wrt_reg_addr)<<endl;
					} else if (func3 == "111") {
						cout<<"\tfunction ANDI..." << endl;
						myRF.writeRF(state.EX.Wrt_reg_addr, myRF.readRF(state.EX.Rs).to_ulong() & to_long(state.EX.Imm));
						cout<<"\tThe instruction is andi R" << state.EX.Wrt_reg_addr.to_ulong()<< ", R"<< state.EX.Rs.to_ulong()<< ", #"<< state.EX.Imm<< endl;
						cout<<"\tData writen in register "<< myRF.readRF(state.EX.Wrt_reg_addr)<<endl;
					}
			} else if (opcode == "1101111") {// JAL <<<<<<<<TODO
				cout<<"\tfunction JAL..." << endl;	
				state.EX.Wrt_reg_addr = bitset<5> (Instruction.to_string().substr(20,5));
				string imm;
				imm.append(Instruction.to_string().substr(0,1));
				imm.append(Instruction.to_string().substr(12,8));
				imm.append(Instruction.to_string().substr(11,1));
				imm.append(Instruction.to_string().substr(1,10));
				imm.append("0");
				cout<<"\tIMM: "<<imm.substr(0,16)<<endl;
				state.EX.Imm = extend(imm.substr(5,16));	
				cout<<"\tCurrent PC "<<state.IF.PC.to_ulong()<<endl;
				cout<<"\tCurrent Imm "<<to_long(state.EX.Imm)<<endl;
				nextState.IF.PC = bitset<32>(state.IF.PC.to_ulong() + to_long(state.EX.Imm));
				cout<<"\tBranching taken to "<<nextState.IF.PC.to_ulong()<<endl;
				myRF.writeRF(state.EX.Wrt_reg_addr, bitset<32>(state.IF.PC.to_ulong() + 4));
                cout<<"\tData writen in register "<< myRF.readRF(state.EX.Wrt_reg_addr)<<endl;
			} 
			if (state.IF.nop) {
				cout<<"\tseting halting to true"<<endl;
               	halted = true; 
			   	ext_dmem.outputDataMem();
            }
			myRF.outputRF(cycle); // dump RF
			printState(nextState, cycle); //print states after executing cycle 0, cycle 1, cycle 2 ... 
			state = nextState; // The end of the cycle and updates the current state with the values calculated in this cycle
			cycle++;
		}

		void printState(stateStruct state, int cycle) {
    		ofstream printstate;
			if (cycle == 0)
				printstate.open(opFilePath, std::ios_base::trunc);
			else 
    			printstate.open(opFilePath, std::ios_base::app);
    		if (printstate.is_open()) {
    		    printstate<<"State after executing cycle:\t"<<cycle<<endl; 

    		    printstate<<"IF.PC:\t"<<state.IF.PC.to_ulong()<<endl;
    		    printstate<<"IF.nop:\t"<<state.IF.nop<<endl;
    		}
    		else cout<<"Unable to open SS StateResult output file." << endl;
    		printstate.close();
		}
	private:
		string opFilePath;
};

class FiveStageCore : public Core{
	public:
		//forwarding   
		string opcode;
		string func;
		bitset<5> Rs;
		bitset<5> Rt;
		bitset<5> Rd; 
    	bitset<16> Imm;
		int counter = 0;

		FiveStageCore(string ioDir, InsMem &imem, DataMem &dmem): Core(ioDir + "\\FS_", imem, dmem), opFilePath(ioDir + "\\StateResult_FS.txt") {}
		
		void step() {
			/* Your implementation */
			cout<<"Cycle:\t"<<cycle<<endl;
			cout<<"PC:\t"<<state.IF.PC.to_ulong()<<endl;  
            cout<<"Halt?: "<< (0? "true":"false") <<endl;
			/* --------------------- WB stage --------------------- */
			cout<<"WB stage:"<<"nop --> " << (state.WB.nop? "true": "false") <<endl;
			if (state.WB.nop == 0){
				myRF.writeRF(state.WB.Wrt_reg_addr, state.WB.Wrt_data);
				cout<<"\tData writen in register "<<state.WB.Wrt_reg_addr.to_ulong()<<": "<< myRF.readRF(state.WB.Wrt_reg_addr)<<endl;      
        	} else {
				nextState.WB.Wrt_reg_addr = bitset<5> (0);
				cout<<"\treseting rd"<<endl;
			}
			/* --------------------- MEM stage -------------------- */
			cout<<"MEM stage:"<<"nop --> " << (state.MEM.nop? "true": "false") <<endl;
			if (0 == state.MEM.nop) {
				//cout<<state.MEM.INS<<endl;
				if (state.MEM.INS == "lw") {
					nextState.WB.Wrt_data = ext_dmem.readDataMem(state.MEM.ALUresult);
					cout<<"\tThe memory at "<< state.MEM.ALUresult <<" is " << nextState.WB.Wrt_data<< endl;
					nextState.WB.Rs = state.MEM.Rs;
					nextState.WB.Wrt_reg_addr = state.MEM.Wrt_reg_addr;
					nextState.WB.wrt_enable = state.MEM.wrt_enable;
				} else if (state.MEM.INS == "add" || state.MEM.INS == "xor" || state.MEM.INS == "or" || state.MEM.INS == "and" || state.MEM.INS == "sub" ) {
					nextState.WB.Wrt_data = state.MEM.ALUresult;
					cout<<"\tPassing ALUresult "<< state.MEM.ALUresult << endl;
					nextState.WB.Rs = state.MEM.Rs;
					nextState.WB.Rt = state.MEM.Rt;
					nextState.WB.Wrt_reg_addr = state.MEM.Wrt_reg_addr;
					nextState.WB.wrt_enable = state.MEM.wrt_enable;
				} else if (state.MEM.INS == "sw") {
					cout<<"\tSkipping WB"<< state.MEM.ALUresult << endl;
					ext_dmem.writeDataMem(state.MEM.ALUresult,myRF.readRF(state.MEM.Rt));
					cout<<"\tsaving "<<myRF.readRF(state.MEM.Rt) << " at "<< state.MEM.ALUresult<<endl;
				} else if (state.MEM.INS == "addi" || state.MEM.INS == "xori" || state.MEM.INS == "ori" || state.MEM.INS == "andi") {
					nextState.WB.Wrt_data = state.MEM.ALUresult;
					cout<<"\tPassing ALUresult "<< state.MEM.ALUresult << endl;
					nextState.WB.Rs = state.MEM.Rs;
					nextState.WB.Rt = state.MEM.Rt;
					nextState.WB.Wrt_reg_addr = state.MEM.Wrt_reg_addr;
					nextState.WB.wrt_enable = state.MEM.wrt_enable;
				} else if (state.MEM.INS == "jal") {
					nextState.WB.Wrt_data = state.MEM.Store_data;
					cout<<"\tPassing Store data "<< state.MEM.ALUresult << endl;
					nextState.WB.Wrt_reg_addr = state.MEM.Wrt_reg_addr;
					nextState.WB.wrt_enable = state.MEM.wrt_enable;
				}
				
			} else {
				nextState.MEM.Wrt_reg_addr = bitset<5> (0);
				cout<<"\treseting rd"<<endl;
			}
			if (!state.MEM.wrt_enable) {
				nextState.WB.nop = true;
			}else nextState.WB.nop = state.MEM.nop;
			/* --------------------- EX stage --------------------- */
			cout<<"EX stage:"<<"nop --> " << (state.EX.nop? "true": "false") <<endl;
			if (0 == state.EX.nop) { 
				if (state.EX.rs1) {
					state.EX.Read_data1 = myRF.readRF(state.EX.Rs);
					cout<<"\treading R"<<state.EX.Rs.to_ulong()<<endl;
				}
				if (state.EX.rs2) {
					state.EX.Read_data2 = myRF.readRF(state.EX.Rt);
					cout<<"\treading R"<<state.EX.Rt.to_ulong()<<endl;
				}
				if (state.EX.INS == "lw") {
					nextState.MEM.ALUresult = bitset<32> (state.EX.Read_data1.to_ulong() + to_long(state.EX.Imm));
					cout<<"\tThe alu result(lw target address) is " << nextState.MEM.ALUresult<< endl;
					nextState.MEM.Rs = state.EX.Rs;
					nextState.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;
					nextState.MEM.rd_mem = state.EX.rd_mem;
					nextState.MEM.wrt_mem = state.EX.wrt_mem;
					nextState.MEM.wrt_enable = state.EX.wrt_enable;
					nextState.MEM.INS = state.EX.INS;
				} else if (state.EX.INS == "add") {
					cout<<"\tThe operation is " <<state.EX.Read_data1<<" + "<<state.EX.Read_data2<< endl;
					nextState.MEM.ALUresult = bitset<32> (state.EX.Read_data1.to_ulong() + state.EX.Read_data2.to_ulong());
					cout<<"\tThe alu result(add result) is " << nextState.MEM.ALUresult<< endl;
					nextState.MEM.Rs = state.EX.Rs;
					nextState.MEM.Rt = state.EX.Rt;
					nextState.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;
					nextState.MEM.rd_mem = state.EX.rd_mem;
					nextState.MEM.wrt_mem = state.EX.wrt_mem;
					nextState.MEM.wrt_enable = state.EX.wrt_enable;
					nextState.MEM.INS = state.EX.INS;
				} else if (state.EX.INS == "sw") {
					cout<<"\tThe operation is " <<state.EX.Read_data1<<" + "<<state.EX.Imm<< endl;
					nextState.MEM.ALUresult = bitset<32> (state.EX.Read_data1.to_ulong() + to_long(state.EX.Imm));
					cout<<"\tThe alu result(sw address) is " << nextState.MEM.ALUresult<< endl;
					nextState.MEM.Rs = state.EX.Rs;
					nextState.MEM.Rt = state.EX.Rt;
					nextState.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;
					nextState.MEM.rd_mem = state.EX.rd_mem;
					nextState.MEM.wrt_mem = state.EX.wrt_mem;
					nextState.MEM.wrt_enable = state.EX.wrt_enable;
					nextState.MEM.INS = state.EX.INS;
				} else if (state.EX.INS == "xor") {
					cout<<"\tThe operation is " <<state.EX.Read_data1<<" ^ "<<state.EX.Read_data2<< endl;
					nextState.MEM.ALUresult = bitset<32> (state.EX.Read_data1.to_ulong() ^ state.EX.Read_data2.to_ulong());
					cout<<"\tThe alu result(xor result) is " << nextState.MEM.ALUresult<< endl;
					nextState.MEM.Rs = state.EX.Rs;
					nextState.MEM.Rt = state.EX.Rt;
					nextState.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;
					nextState.MEM.rd_mem = state.EX.rd_mem;
					nextState.MEM.wrt_mem = state.EX.wrt_mem;
					nextState.MEM.wrt_enable = state.EX.wrt_enable;
					nextState.MEM.INS = state.EX.INS;
				} else if (state.EX.INS == "or") {
					cout<<"\tThe operation is " <<state.EX.Read_data1<<" | "<<state.EX.Read_data2<< endl;
					nextState.MEM.ALUresult = bitset<32> (state.EX.Read_data1.to_ulong() | state.EX.Read_data2.to_ulong());
					cout<<"\tThe alu result(or result) is " << nextState.MEM.ALUresult<< endl;
					nextState.MEM.Rs = state.EX.Rs;
					nextState.MEM.Rt = state.EX.Rt;
					nextState.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;
					nextState.MEM.rd_mem = state.EX.rd_mem;
					nextState.MEM.wrt_mem = state.EX.wrt_mem;
					nextState.MEM.wrt_enable = state.EX.wrt_enable;
					nextState.MEM.INS = state.EX.INS;
				} else if (state.EX.INS == "and") {
					cout<<"\tThe operation is " <<state.EX.Read_data1<<" & "<<state.EX.Read_data2<< endl;
					nextState.MEM.ALUresult = bitset<32> (state.EX.Read_data1.to_ulong() & state.EX.Read_data2.to_ulong());
					cout<<"\tThe alu result(and result) is " << nextState.MEM.ALUresult<< endl;
					nextState.MEM.Rs = state.EX.Rs;
					nextState.MEM.Rt = state.EX.Rt;
					nextState.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;
					nextState.MEM.rd_mem = state.EX.rd_mem;
					nextState.MEM.wrt_mem = state.EX.wrt_mem;
					nextState.MEM.wrt_enable = state.EX.wrt_enable;
					nextState.MEM.INS = state.EX.INS;
				} else if (state.EX.INS == "sub") {
					cout<<"\tThe operation is " <<state.EX.Read_data1<<" - "<<state.EX.Read_data2<< endl;
					nextState.MEM.ALUresult = bitset<32> (state.EX.Read_data1.to_ulong() - state.EX.Read_data2.to_ulong());
					cout<<"\tThe alu result(sub result) is " << nextState.MEM.ALUresult<< endl;
					nextState.MEM.Rs = state.EX.Rs;
					nextState.MEM.Rt = state.EX.Rt;
					nextState.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;
					nextState.MEM.rd_mem = state.EX.rd_mem;
					nextState.MEM.wrt_mem = state.EX.wrt_mem;
					nextState.MEM.wrt_enable = state.EX.wrt_enable;
					nextState.MEM.INS = state.EX.INS;
				} else if (state.EX.INS == "addi") {
					cout<<"\tThe operation is " <<state.EX.Read_data1<<" + #"<<state.EX.Imm<< endl;
					nextState.MEM.ALUresult = bitset<32> (state.EX.Read_data1.to_ulong() + to_long(state.EX.Imm));
					cout<<"\tThe alu result(addi result) is " << nextState.MEM.ALUresult<< endl;
					nextState.MEM.Rs = state.EX.Rs;
					nextState.MEM.Rt = state.EX.Rt;
					nextState.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;
					nextState.MEM.rd_mem = state.EX.rd_mem;
					nextState.MEM.wrt_mem = state.EX.wrt_mem;
					nextState.MEM.wrt_enable = state.EX.wrt_enable;
					nextState.MEM.INS = state.EX.INS;
				} else if (state.EX.INS == "xori") {
					cout<<"\tThe operation is " <<state.EX.Read_data1<<" ^ #"<<state.EX.Imm<< endl;
					nextState.MEM.ALUresult = bitset<32> (state.EX.Read_data1.to_ulong() ^ to_long(state.EX.Imm));
					cout<<"\tThe alu result(xori result) is " << nextState.MEM.ALUresult<< endl;
					nextState.MEM.Rs = state.EX.Rs;
					nextState.MEM.Rt = state.EX.Rt;
					nextState.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;
					nextState.MEM.rd_mem = state.EX.rd_mem;
					nextState.MEM.wrt_mem = state.EX.wrt_mem;
					nextState.MEM.wrt_enable = state.EX.wrt_enable;
					nextState.MEM.INS = state.EX.INS;
				} else if (state.EX.INS == "ori") {
					cout<<"\tThe operation is " <<state.EX.Read_data1<<" | #"<<state.EX.Imm<< endl;
					nextState.MEM.ALUresult = bitset<32> (state.EX.Read_data1.to_ulong() | to_long(state.EX.Imm));
					cout<<"\tThe alu result(ori result) is " << nextState.MEM.ALUresult<< endl;
					nextState.MEM.Rs = state.EX.Rs;
					nextState.MEM.Rt = state.EX.Rt;
					nextState.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;
					nextState.MEM.rd_mem = state.EX.rd_mem;
					nextState.MEM.wrt_mem = state.EX.wrt_mem;
					nextState.MEM.wrt_enable = state.EX.wrt_enable;
					nextState.MEM.INS = state.EX.INS;
				} else if (state.EX.INS == "andi") {
					cout<<"\tThe operation is " <<state.EX.Read_data1<<" & #"<<state.EX.Imm<< endl;
					nextState.MEM.ALUresult = bitset<32> (state.EX.Read_data1.to_ulong() & to_long(state.EX.Imm));
					cout<<"\tThe alu result(andi result) is " << nextState.MEM.ALUresult<< endl;
					nextState.MEM.Rs = state.EX.Rs;
					nextState.MEM.Rt = state.EX.Rt;
					nextState.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;
					nextState.MEM.rd_mem = state.EX.rd_mem;
					nextState.MEM.wrt_mem = state.EX.wrt_mem;
					nextState.MEM.wrt_enable = state.EX.wrt_enable;
					nextState.MEM.INS = state.EX.INS;
				} else if (state.EX.INS == "jal") {
					nextState.MEM.Store_data = state.EX.PC;
					cout<<"\tjal passing by" << nextState.MEM.Store_data<< endl;
					nextState.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;
					nextState.MEM.rd_mem = state.EX.rd_mem;
					nextState.MEM.wrt_mem = state.EX.wrt_mem;
					nextState.MEM.wrt_enable = state.EX.wrt_enable;
					nextState.MEM.INS = state.EX.INS;
				}
			} else {
				nextState.EX.Wrt_reg_addr = bitset<5> (0);
				cout<<"\treseting rd"<<endl;
			}
			if (!state.EX.wrt_enable && !state.EX.rd_mem) {
				nextState.MEM.nop = true;
			}else nextState.MEM.nop = state.EX.nop;
			/* --------------------- ID stage --------------------- */
			bool branch = false;
			bool stallRs1 = false;
			bool stallRs2 = false;
			nextState.EX.rs1 = false;
			nextState.EX.rs2 = false;
			cout<<"ID stage:nop --> " << (state.ID.nop? "true": "false") <<endl;
			if (0 == state.ID.nop) {
				bitset<32> Instruction = state.ID.Instr;
				cout<<"\tThe instruction is " << Instruction << endl;	
				opcode = Instruction.to_string().substr(25,7);
				cout<<"\tThe opcode is " << opcode << endl;
				if (opcode == "0000011") {// LW
					//read
					nextState.EX.INS = "lw"; 
					nextState.EX.Imm = extend(Instruction.to_string().substr(0,12));
                	nextState.EX.Rs = bitset<5> (Instruction.to_string().substr(12,5));
                	nextState.EX.Wrt_reg_addr = bitset<5> (Instruction.to_string().substr(20,5));
					cout<<"\tfunction is lw R"<<nextState.EX.Wrt_reg_addr.to_ulong()<<", R"<< nextState.EX.Rs.to_ulong()<<", #"<<nextState.EX.Imm<< endl;
					nextState.EX.wrt_enable = 1;
					nextState.EX.wrt_mem = 0;
					nextState.EX.rd_mem = 1;
					nextState.EX.Read_data1 = myRF.readRF(nextState.EX.Rs);
					int r1 = nextState.EX.Rs.to_ulong();
					if (r1!= 0) {// R0 is always 0
						if(!nextState.MEM.nop && nextState.MEM.Wrt_reg_addr.to_ulong() == r1){
							cout<<"\tHazard detected"<<endl;
							stallRs1 = true;
						}
					}
				} else if (opcode == "0100011") {// SW
					nextState.EX.INS = "sw"; 
					nextState.EX.Rs = bitset<5> (Instruction.to_string().substr(12,5));
                    nextState.EX.Rt = bitset<5> (Instruction.to_string().substr(7,5));
					nextState.EX.Imm = extend(Instruction.to_string().substr(0,7).append(Instruction.to_string().substr(20,5)));
					cout<<"\tThe instruction is sw R" << nextState.EX.Rt.to_ulong()<< ", R"<< nextState.EX.Rs.to_ulong()<< ", #"<< nextState.EX.Imm<< endl;
					int r1 = nextState.EX.Rs.to_ulong();
					if (r1!= 0) {// R0 is always 0
						if(!nextState.MEM.nop && nextState.MEM.Wrt_reg_addr.to_ulong() == r1){
							cout<<"\tHazard detected"<<endl;
							cout<<"\trs1: "<<r1<<endl;
							stallRs1 = true;
						}
						if(!nextState.WB.nop && nextState.WB.Wrt_reg_addr.to_ulong() == r1) {
							nextState.EX.rs1 = true;
						}
					}
					int r2 = nextState.EX.Rt.to_ulong();
					if (r2!= 0) {// R0 is always 0
						if(!nextState.MEM.nop && nextState.MEM.Wrt_reg_addr.to_ulong() == r2){
							cout<<"\tHazard detected"<<endl;
							cout<<"\trs2: "<<r2<<endl;
							stallRs2 = true;
						}
						if(!nextState.WB.nop && nextState.WB.Wrt_reg_addr.to_ulong() == r2) {
							nextState.EX.rs2 = true;
						}
					}
					if(!stallRs1 && !stallRs2) {
						nextState.EX.wrt_enable = 0;
						nextState.EX.wrt_mem = 1;
						nextState.EX.rd_mem = 0;
						nextState.EX.Read_data1 = myRF.readRF(nextState.EX.Rs);
						cout<<"\tdata1 passed: "<<myRF.readRF(nextState.EX.Rs)<<endl;
						nextState.EX.Read_data2 = myRF.readRF(nextState.EX.Rt);
						cout<<"\tdata2 passed: "<<myRF.readRF(nextState.EX.Rt)<<endl;
					} else {
						cout<<"\tWB rd: "<<nextState.WB.Wrt_reg_addr.to_ulong()<<endl;
						cout<<"\tMEM rd: "<<nextState.MEM.Wrt_reg_addr.to_ulong()<<endl;
					}
				} else if (opcode == "1100011") {// BNE or BEQ
					nextState.EX.Rs = bitset<5> (Instruction.to_string().substr(12,5));
                    nextState.EX.Rt = bitset<5> (Instruction.to_string().substr(7,5));
					string imm;
					imm.append(Instruction.to_string().substr(0,1));
					imm.append(Instruction.to_string().substr(24,1));
					imm.append(Instruction.to_string().substr(1,6));
					imm.append(Instruction.to_string().substr(20,4));
					imm.append("0");
					nextState.EX.Imm = extend(imm);
					string func3 = Instruction.to_string().substr(17,3);
					cout<<"\tThe func3 is " << func3 << endl;
					cout<<"\tThe instruction is "<<((func3 == "000")? "beq" : "bne")<<" R" << nextState.EX.Rt.to_ulong()<< ", R"<< nextState.EX.Rs.to_ulong()<< ", #"<< to_long(nextState.EX.Imm)<< endl;
					
					int r1 = nextState.EX.Rs.to_ulong();
					if (r1!= 0) {// R0 is always 0
						if(!nextState.MEM.nop && nextState.MEM.Wrt_reg_addr.to_ulong() == r1){
							cout<<"\tHazard detected"<<endl;
							cout<<"\trs1: "<<r1<<endl;
							stallRs1 = true;
						}
						if(!nextState.WB.nop && nextState.WB.Wrt_reg_addr.to_ulong() == r1) {
							nextState.EX.rs1 = true;
						}
					}
					int r2 = nextState.EX.Rt.to_ulong();
					if (r2!= 0) {// R0 is always 0
						if(!nextState.MEM.nop && nextState.MEM.Wrt_reg_addr.to_ulong() == r2){
							cout<<"\tHazard detected"<<endl;
							cout<<"\trs2: "<<r2<<endl;
							stallRs2 = true;
						}
						if(!nextState.WB.nop && nextState.WB.Wrt_reg_addr.to_ulong() == r2) {
							nextState.EX.rs2 = true;
						}
					}
					if(!stallRs1 && !stallRs2) {
						nextState.EX.wrt_enable = 0;
						nextState.EX.wrt_mem = 0;
						nextState.EX.rd_mem = 0;
						nextState.EX.Read_data1 = myRF.readRF(nextState.EX.Rs);
						nextState.EX.Read_data2 = myRF.readRF(nextState.EX.Rt);
					} else {
						cout<<"\tWB rd: "<<nextState.WB.Wrt_reg_addr.to_ulong()<<endl;
						cout<<"\tMEM rd: "<<nextState.MEM.Wrt_reg_addr.to_ulong()<<endl;
					}
					//forwarding in ID MEM stage 
					if(state.MEM.Wrt_reg_addr.to_ulong() == nextState.EX.Rs.to_ulong()) {
						cout<<"\tGetting RS1 from MEM "<<state.MEM.ALUresult<<endl;
						nextState.EX.Read_data1 = state.MEM.ALUresult;
					}
					if(state.MEM.Wrt_reg_addr.to_ulong() == nextState.EX.Rt.to_ulong()) {
						cout<<"\tGetting RS1 from MEM "<<state.MEM.ALUresult<<endl;
						nextState.EX.Read_data2 = state.MEM.ALUresult;
					}
					cout<<"\tdata1 passed: "<<nextState.EX.Read_data1<<endl;
					cout<<"\tdata2 passed: "<<nextState.EX.Read_data2<<endl;
					cout<<"\tStall RS 1 "<<stallRs1<<endl;
					cout<<"\tStall RS 2 "<<stallRs2<<endl;
					cout<<"the operation is "<<nextState.EX.Read_data1.to_ulong()<<" comprared with "<< nextState.EX.Read_data2.to_ulong()<<endl;
					if(!stallRs1 && !stallRs2) {
						if (func3 == "000") {//BEQ
							if (nextState.EX.Read_data1.to_ulong() == nextState.EX.Read_data2.to_ulong()) {
								cout<<"\tCurrent PC "<<state.ID.PC.to_ulong()<<endl;
								cout<<"\tCurrent Imm "<<to_long(nextState.EX.Imm)<<endl;
								state.IF.PC = bitset<32>(state.ID.PC.to_ulong() + to_long(nextState.EX.Imm));
								cout<<"\tBranching taken to "<<state.IF.PC.to_ulong()<<endl;
								branch = true;
							}else cout<<"\tNo Branching taken "<<endl;
						} else if(func3 == "001"){//BNE
							if (nextState.EX.Read_data1.to_ulong() != nextState.EX.Read_data2.to_ulong()) {
								cout<<"\tCurrent PC "<<state.ID.PC.to_ulong()<<endl;
								cout<<"\tCurrent Imm "<<to_long(nextState.EX.Imm)<<endl;
								state.IF.PC = bitset<32>(state.ID.PC.to_ulong() + to_long(nextState.EX.Imm));
								cout<<"\tBranching taken to "<<state.IF.PC.to_ulong()<<endl;
								branch = true;
							}else cout<<"\tNo Branching taken "<<endl;
						}
					}
					
					
				} else if (opcode == "0110011") {// ADD or SUB or XOR or OR or AND
					string func7 = Instruction.to_string().substr(0,7);
					cout<<"\tThe func7 is " << func7 << endl;
					nextState.EX.Rs = bitset<5> (Instruction.to_string().substr(12,5));
                    nextState.EX.Rt = bitset<5> (Instruction.to_string().substr(7,5));
                    nextState.EX.Wrt_reg_addr = bitset<5> (Instruction.to_string().substr(20,5)); 
					if (func7 == "0000000") {
						string func3 = Instruction.to_string().substr(17,3);
						cout<<"\tThe func3 is " << func3 << endl;
						
						if (func3 == "000") {//ADD
							nextState.EX.INS = "add";
							cout<<"\tfunction is add R"<<nextState.EX.Wrt_reg_addr.to_ulong()<<", R"<< nextState.EX.Rs.to_ulong()<<", R"<< nextState.EX.Rt.to_ulong()<< endl;
						} else if (func3 == "100") {//XOR
							nextState.EX.INS = "xor";
							cout<<"\tfunction is xor R"<<nextState.EX.Wrt_reg_addr.to_ulong()<<", R"<< nextState.EX.Rs.to_ulong()<<", R"<< nextState.EX.Rt.to_ulong()<< endl;
						} else if (func3 == "110") {//OR
							nextState.EX.INS = "or";
							cout<<"\tfunction is or R"<<nextState.EX.Wrt_reg_addr.to_ulong()<<", R"<< nextState.EX.Rs.to_ulong()<<", R"<< nextState.EX.Rt.to_ulong()<< endl;
						} else if (func3 == "111") {//AND
							nextState.EX.INS = "and"; 
							cout<<"\tfunction is and R"<<nextState.EX.Wrt_reg_addr.to_ulong()<<", R"<< nextState.EX.Rs.to_ulong()<<", R"<< nextState.EX.Rt.to_ulong()<< endl;
						}
					} else if (func7 == "0100000") {//SUB
						nextState.EX.INS = "sub"; 
						cout<<"\tfunction is sub R"<<nextState.EX.Wrt_reg_addr.to_ulong()<<", R"<< nextState.EX.Rs.to_ulong()<<", R"<< nextState.EX.Rt.to_ulong()<< endl;
					}
					int r1 = nextState.EX.Rs.to_ulong();
					if (r1!= 0) {// R0 is always 0
						if(!nextState.MEM.nop && nextState.MEM.Wrt_reg_addr.to_ulong() == r1){
							cout<<"\tHazard detected"<<endl;
							cout<<"\trs1: "<<r1<<endl;
							stallRs1 = true;
						}
						if(!nextState.WB.nop && nextState.WB.Wrt_reg_addr.to_ulong() == r1) {
							nextState.EX.rs1 = true;
						}
					}
					int r2 = nextState.EX.Rt.to_ulong();
					if (r2!= 0) {// R0 is always 0
						if(!nextState.MEM.nop && nextState.MEM.Wrt_reg_addr.to_ulong() == r2){
							cout<<"\tHazard detected"<<endl;
							cout<<"\trs2: "<<r2<<endl;
							stallRs2 = true;
						}
						if(!nextState.WB.nop && nextState.WB.Wrt_reg_addr.to_ulong() == r2) {
							nextState.EX.rs2 = true;
						}
					}
					if(!stallRs1 && !stallRs2) {
						nextState.EX.wrt_enable = 1;
						nextState.EX.wrt_mem = 0;
						nextState.EX.rd_mem = 0;
						nextState.EX.Read_data1 = myRF.readRF(nextState.EX.Rs);
						cout<<"\tdata1 passed: "<<myRF.readRF(nextState.EX.Rs)<<endl;
						nextState.EX.Read_data2 = myRF.readRF(nextState.EX.Rt);
						cout<<"\tdata2 passed: "<<myRF.readRF(nextState.EX.Rt)<<endl;
					} else {
						cout<<"\tWB rd: "<<nextState.WB.Wrt_reg_addr.to_ulong()<<endl;
						cout<<"\tMEM rd: "<<nextState.MEM.Wrt_reg_addr.to_ulong()<<endl;
					}
				} else if (opcode == "0010011") {// ADDI or XORI or ORI or ANDI
					nextState.EX.Rs = bitset<5> (Instruction.to_string().substr(12,5));
					nextState.EX.Imm = extend(Instruction.to_string().substr(0,12));
                    nextState.EX.Wrt_reg_addr = bitset<5> (Instruction.to_string().substr(20,5)); 
					string func3 = Instruction.to_string().substr(17,3);
					cout<<"\tThe func3 is " << func3 << endl;
					if (func3 == "000") {
						nextState.EX.INS = "addi";
						cout<<"\tThe instruction is addi R" << nextState.EX.Wrt_reg_addr.to_ulong()<< ", R"<< nextState.EX.Rs.to_ulong()<< ", #"<< nextState.EX.Imm<< endl;
					} else if (func3 == "100") {
						nextState.EX.INS = "xori";
						cout<<"\tThe instruction is xori R" << nextState.EX.Wrt_reg_addr.to_ulong()<< ", R"<< nextState.EX.Rs.to_ulong()<< ", #"<< nextState.EX.Imm<< endl;
					} else if (func3 == "110") {
						nextState.EX.INS = "ori";
						cout<<"\tThe instruction is ori R" << nextState.EX.Wrt_reg_addr.to_ulong()<< ", R"<< nextState.EX.Rs.to_ulong()<< ", #"<< nextState.EX.Imm<< endl;
					} else if (func3 == "111") {
						nextState.EX.INS = "andi";
						cout<<"\tThe instruction is andi R" << nextState.EX.Wrt_reg_addr.to_ulong()<< ", R"<< nextState.EX.Rs.to_ulong()<< ", #"<< nextState.EX.Imm<< endl;
					}
					nextState.EX.wrt_enable = 1;
					nextState.EX.wrt_mem = 0;
					nextState.EX.rd_mem = 0;
					nextState.EX.Read_data1 = myRF.readRF(nextState.EX.Rs);
					int r1 = nextState.EX.Rs.to_ulong();
					if (r1!= 0) {// R0 is always 0
						if(!nextState.MEM.nop && nextState.MEM.Wrt_reg_addr.to_ulong() == r1){
							cout<<"\tHazard detected"<<endl;
							stallRs1 = true;
						}
					}
				} else if (opcode == "1101111") {// JAL
					nextState.EX.INS = "jal";
					cout<<"\tThe instruction is jal R" << nextState.EX.Wrt_reg_addr.to_ulong()<<", #"<< nextState.EX.Imm<< endl;
					nextState.EX.Wrt_reg_addr = bitset<5> (Instruction.to_string().substr(20,5)); 
					string imm;
					imm.append(Instruction.to_string().substr(0,1));
					imm.append(Instruction.to_string().substr(12,8));
					imm.append(Instruction.to_string().substr(11,1));
					imm.append(Instruction.to_string().substr(1,10));
					imm.append("0");
					nextState.EX.Imm = extend(imm.substr(5,16)); 
					cout<<"\tCurrent PC "<<state.ID.PC.to_ulong()<<endl;
					cout<<"\tCurrent Imm "<<to_long(nextState.EX.Imm)<<endl;
					state.IF.PC = bitset<32>(state.ID.PC.to_ulong() + to_long(nextState.EX.Imm));
					cout<<"\tBranching taken to "<<state.IF.PC.to_ulong()<<endl;
					branch = true;		
					nextState.EX.wrt_enable = 1;
					nextState.EX.wrt_mem = 0;
					nextState.EX.rd_mem = 0;
					nextState.EX.PC = state.ID.PC.to_ulong() + 4;
				}
        	}
        	if (!stallRs1 && !stallRs2) nextState.EX.nop = state.ID.nop;
			else nextState.EX.nop = true;
			/* --------------------- IF stage --------------------- */
			cout<<"IF stage:orginal nop --> " << (state.IF.nop? "true": "false") <<endl;
			cout<<"\tBranch IF "<<(branch? "true": "false")<<endl;
			bitset<32> Instruction= ext_imem.readInstr(state.IF.PC);
			cout<<"\tInstruction fetched:\t"<<Instruction<<" from "<<state.IF.PC.to_ulong()<<endl;
			if (Instruction != 0xffffffff) {
				nextState.IF.PC = state.IF.PC.to_ulong() + 4;
				state.IF.nop = false;
				nextState.IF.nop = false;                               
			} else {// HALTING
				if(branch) {
					state.IF.nop = false; 
					nextState.IF.PC = state.IF.PC.to_ulong();
				} else {
					state.IF.nop = true;
					nextState.IF.PC = state.IF.PC.to_ulong();                
					nextState.IF.nop = true;
				}
				              
			} 
			cout<<"\tnop --> " << (state.IF.nop? "true": "false") <<endl;
			if(!stallRs1 && !stallRs2){
				if (0 == state.IF.nop) { 
					nextState.ID.Instr = Instruction;   
					nextState.ID.PC = state.IF.PC; 
        		}
        		nextState.ID.nop = state.IF.nop;
			} else {
				nextState.IF.PC = state.IF.PC;
				cout<<"\trepeating PC " << nextState.IF.PC.to_ulong()<< " because of Hazard"<<endl;
			}
			/* --------------------- End for stages --------------------- */
			cout<<"PC:\t "<<state.IF.PC.to_ulong()<<endl;  
			cout<<"next PC: "<<nextState.IF.PC.to_ulong()<<endl; 
			if (state.IF.PC.to_ulong() != nextState.IF.PC.to_ulong()) {
				counter++;
			}
			cout<<"Total execution cycle  "<<cycle + 1.0<<endl;
			cout<<"average CPI "<<(cycle + 1.0)/counter<<endl;
			cout<<"average IPC "<<counter/(cycle + 1.0)<<endl;
			if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop){
				halted = true;
				ext_dmem.outputDataMem();
			}
            myRF.outputRF(cycle); // dump RF
			printState(nextState, cycle); //print states after executing cycle 0, cycle 1, cycle 2 ... 
       
			state = nextState; //The end of the cycle and updates the current state with the values calculated in this cycle
			cycle++;
		}

		void printState(stateStruct state, int cycle) {
		    ofstream printstate;
			if (cycle == 0)
				printstate.open(opFilePath, std::ios_base::trunc);
			else 
		    	printstate.open(opFilePath, std::ios_base::app);
		    if (printstate.is_open()) {
		        printstate<<"State after executing cycle:\t"<<cycle<<endl; 

		        printstate<<"IF.PC:\t"<<state.IF.PC.to_ulong()<<endl;        
		        printstate<<"IF.nop:\t"<<state.IF.nop<<endl; 

		        printstate<<"ID.Instr:\t"<<state.ID.Instr<<endl; 
		        printstate<<"ID.nop:\t"<<state.ID.nop<<endl;

		        printstate<<"EX.Read_data1:\t"<<state.EX.Read_data1<<endl;
		        printstate<<"EX.Read_data2:\t"<<state.EX.Read_data2<<endl;
		        printstate<<"EX.Imm:\t"<<state.EX.Imm<<endl; 
		        printstate<<"EX.Rs:\t"<<state.EX.Rs<<endl;
		        printstate<<"EX.Rt:\t"<<state.EX.Rt<<endl;
		        printstate<<"EX.Wrt_reg_addr:\t"<<state.EX.Wrt_reg_addr<<endl;
		        printstate<<"EX.rd_mem:\t"<<state.EX.rd_mem<<endl;
		        printstate<<"EX.wrt_mem:\t"<<state.EX.wrt_mem<<endl;        
		        printstate<<"EX.alu_op:\t"<<state.EX.alu_op<<endl;
		        printstate<<"EX.wrt_enable:\t"<<state.EX.wrt_enable<<endl;
		        printstate<<"EX.nop:\t"<<state.EX.nop<<endl;        

		        printstate<<"MEM.ALUresult:\t"<<state.MEM.ALUresult<<endl;
		        printstate<<"MEM.Store_data:\t"<<state.MEM.Store_data<<endl; 
		        printstate<<"MEM.Rs:\t"<<state.MEM.Rs<<endl;
		        printstate<<"MEM.Rt:\t"<<state.MEM.Rt<<endl;   
		        printstate<<"MEM.Wrt_reg_addr:\t"<<state.MEM.Wrt_reg_addr<<endl;              
		        printstate<<"MEM.rd_mem:\t"<<state.MEM.rd_mem<<endl;
		        printstate<<"MEM.wrt_mem:\t"<<state.MEM.wrt_mem<<endl; 
		        printstate<<"MEM.wrt_enable:\t"<<state.MEM.wrt_enable<<endl;         
		        printstate<<"MEM.nop:\t"<<state.MEM.nop<<endl;        

		        printstate<<"WB.Wrt_data:\t"<<state.WB.Wrt_data<<endl;
		        printstate<<"WB.Rs:\t"<<state.WB.Rs<<endl;
		        printstate<<"WB.Rt:\t"<<state.WB.Rt<<endl;
		        printstate<<"WB.Wrt_reg_addr:\t"<<state.WB.Wrt_reg_addr<<endl;
		        printstate<<"WB.wrt_enable:\t"<<state.WB.wrt_enable<<endl;
		        printstate<<"WB.nop:\t"<<state.WB.nop<<endl; 
		    }
		    else cout<<"Unable to open FS StateResult output file." << endl;
		    printstate.close();
		}
	private:
		string opFilePath;
};

int main(int argc, char* argv[]) {
	
	string ioDir = "";
    if (argc == 1) {
        cout << "Enter path containing the memory files: ";
        cin >> ioDir;
    }
    else if (argc > 2) {
        cout << "Invalid number of arguments. Machine stopped." << endl;
        return -1;
    }
    else {
        ioDir = argv[1];
        cout << "IO Directory: " << ioDir << endl;
    }

    InsMem imem = InsMem("Imem", ioDir);
    DataMem dmem_ss = DataMem("SS", ioDir);
	DataMem dmem_fs = DataMem("FS", ioDir);

	SingleStageCore SSCore(ioDir, imem, dmem_ss);
	FiveStageCore FSCore(ioDir, imem, dmem_fs);

    while (1) {
        cout<<"-----------------------------------------"<<endl;
        //cout << "SShalted?: "<< (SSCore.halted? "true":"false")<< endl;
        //cout << "FShalted?: "<< (FSCore.halted? "true":"false")<< endl;
		if (!SSCore.halted) {
            cout<<"----Single Stage Core log"<<endl;
			SSCore.step();
			cout<<"Total execution cycle "<<SSCore.cycle<<endl;
			cout<<"Average CPI 1"<<endl;
			cout<<"Average IPC 1"<<endl;
        }
		if (!FSCore.halted) {
			cout<<"----Five Stage Core log"<<endl;
			FSCore.step();
		}

		if (SSCore.halted && FSCore.halted)
			break;
    }
	return 0;
}