#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>
using namespace std;

#define MemSize 1000 // memory size, in reality, the memory size should be 2^32, but for this lab, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.

struct IFStruct {
    bitset<32>  PC;
    bool        nop;  
};

struct IDStruct {
    bitset<32>  Instr;
    bool        nop;  
};

struct EXStruct {
    bitset<32>  Read_data1;
    bitset<32>  Read_data2;
    bitset<16>  Imm;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bool        is_I_type;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        alu_op;     //1 for addu, lw, sw, 0 for subu 
    bool        wrt_enable;
    bool        nop;
    bitset<32>  INS;   
};

struct MEMStruct {
    bitset<32>  ALUresult;
    bitset<32>  Store_data;
    bitset<5>   Rs;
    bitset<5>   Rt;    
    bitset<5>   Wrt_reg_addr;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        wrt_enable;    
    bool        nop;
    bitset<32>  INS;     
};

struct WBStruct {
    bitset<32>  Wrt_data;
    bitset<5>   Rs;
    bitset<5>   Rt;     
    bitset<5>   Wrt_reg_addr;
    bool        wrt_enable;
    bool        nop;
    bitset<32>  INS;   
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
        bitset<32> Instruction;
        InsMem(string name, string ioDir) {       
			id = name;
			IMem.resize(MemSize);
            ifstream imem;
			string line;
			int i=0;
			imem.open(ioDir + "/imem.txt");
			if (imem.is_open())
			{
				while (getline(imem,line)) {      
					IMem[i] = bitset<8>(line);
					i++;
				}                    
			}
            else cout<<"Unable to open IMEM input file.";
			imem.close();                     
		}

		bitset<32> readInstr(bitset<32> Address) {    
			string instruction;
            instruction.append(IMem[Address.to_ulong()].to_string());
            instruction.append(IMem[Address.to_ulong()+1].to_string());
            instruction.append(IMem[Address.to_ulong()+2].to_string());
            instruction.append(IMem[Address.to_ulong()+3].to_string());
            return bitset<32>(instruction); 
		}     
      
    private:
        vector<bitset<8>> IMem;     
};
      
class DataMem    
{
    public: 
		string id, opFilePath, ioDir;
        bitset<32> ReadData;
        DataMem(string name, string ioDir) : id(name), ioDir(ioDir) {
            DMem.resize(MemSize);
			opFilePath = ioDir + "/" + name + "_DMEMResult.txt";
            ifstream dmem;
            string line;
            int i=0;
            dmem.open(ioDir + "/dmem.txt");
            if (dmem.is_open())
            {
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
			string datamem;
            datamem.append(DMem[Address.to_ulong()].to_string());
            datamem.append(DMem[Address.to_ulong()+1].to_string());
            datamem.append(DMem[Address.to_ulong()+2].to_string());
            datamem.append(DMem[Address.to_ulong()+3].to_string());
            ReadData = bitset<32>(datamem);
            return ReadData;        
		}
            
        void writeDataMem(bitset<32> Address, bitset<32> WriteData) {
			// write into memory
            DMem[Address.to_ulong()] = bitset<8>(WriteData.to_string().substr(0, 8));
            DMem[Address.to_ulong() + 1] = bitset<8>(WriteData.to_string().substr(8, 8));
            DMem[Address.to_ulong() + 2] = bitset<8>(WriteData.to_string().substr(16, 8));
            DMem[Address.to_ulong() + 3] = bitset<8>(WriteData.to_string().substr(24, 8));
        }   
                     
        void outputDataMem() {
            ofstream dmemout;
            dmemout.open(opFilePath, std::ios_base::trunc);
            if (dmemout.is_open()) {
                for (int j = 0; j< 1000; j++)
                {     
                    dmemout << DMem[j]<<endl;
                }
                     
            }
            else cout<<"Unable to open "<<id<<" DMEM result file." << endl;
            dmemout.close();
        }             

    private:
		vector<bitset<8> > DMem;      
};

class RegisterFile
{
    public:
		string outputFile;
        bitset<32> Reg_data;
     	RegisterFile(string ioDir): outputFile(ioDir + "RFResult.txt") {
			Registers.resize(32);  
			Registers[0] = bitset<32> (0);
        }
	
        bitset<32> readRF(bitset<5> Reg_addr) {   
            // Fill in
            Reg_data = Registers[Reg_addr.to_ulong()];
            return Reg_data;
        }
    
        void writeRF(bitset<5> Reg_addr, bitset<32> Wrt_reg_data) {
            // Fill in
            Registers[Reg_addr.to_ulong()] = Wrt_reg_data;

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
bitset<32> sign_extend(bitset<16> Imm) {
    string se0 = "0000000000000000";
    string se1 = "1111111111111111";
    string immediate = Imm.to_string();
    bitset<32> se_imm;
    
    if (immediate.substr(0,1) == "0") {
        se0 += immediate;
        se_imm = bitset<32>(se0);
    } else {
        se1.append(immediate);
        se_imm = bitset<32>(se1);				
    }
    return se_imm;
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
		
		Core(string ioDir, InsMem &imem, DataMem &dmem): myRF(ioDir), ioDir(ioDir), ext_imem(imem), ext_dmem(dmem) {}

		virtual void step() {}

		virtual void printState() {}
};

class SingleStageCore : public Core {
	public:
		SingleStageCore(string ioDir, InsMem &imem, DataMem &dmem): Core(ioDir + "/SS_", imem, dmem), opFilePath(ioDir + "/StateResult_SS.txt") {}

		void step() {
			/* Your implementation*/
			//step 1 fetch ins:
			// get instruction using read intruction from PC from IF from state
			//step 2 execute instruction:
			// helper: identify and execute the instuction given: 32bit instruction
			//step 3 update state:
			// update nop in next state by certain condition
			halted = true;//stub
			if (state.IF.nop)
				halted = true;
			
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
		
		FiveStageCore(string ioDir, InsMem &imem, DataMem &dmem): Core(ioDir + "/FS_", imem, dmem), opFilePath(ioDir + "/StateResult_FS.txt") {}

		void step() {
			/* Your implementation */
			/* --------------------- WB stage --------------------- */
			if (0 == state.WB.nop) {
            	if (1 == state.WB.wrt_enable) {
                	myRF.writeRF(state.WB.Wrt_reg_addr, state.WB.Wrt_data);
            	}         
            }
			/* --------------------- MEM stage -------------------- */
			if (0 == state.MEM.nop) {
            if (1 == state.MEM.rd_mem) {
                nextState.WB.Wrt_data = ext_dmem.readDataMem(state.MEM.ALUresult);
            } else if (1 == state.MEM.wrt_mem) {
                if ((0 == state.WB.nop) && (1 == state.WB.wrt_enable) && (state.WB.Wrt_reg_addr == state.MEM.Rt))
                {
                    state.MEM.Store_data = state.WB.Wrt_data ;    
                    cout<<"MEM-MEM sw Forwarding"<<endl;
                }
                    
                ext_dmem.writeDataMem(state.MEM.ALUresult, state.MEM.Store_data);
                nextState.WB.Wrt_data = state.MEM.Store_data;    //will not be used
            } else if (1 == state.MEM.wrt_enable) {
                //cout<<"addu subu ALUresult:\t"<<state.MEM.ALUresult<<endl;
                nextState.WB.Wrt_data = state.MEM.ALUresult;
            }   
            nextState.WB.Rs = state.MEM.Rs;
            nextState.WB.Rt = state.MEM.Rt;             
            nextState.WB.Wrt_reg_addr = state.MEM.Wrt_reg_addr;                      
            nextState.WB.wrt_enable = state.MEM.wrt_enable;             
            }
        	nextState.WB.nop = state.MEM.nop;  
        	nextState.WB.INS = state.MEM.INS;


        	/* --------------------- EX stage --------------------- */
        	if (0 == state.EX.nop) {                       
            	if ((0 == state.WB.nop) && (1 == state.WB.wrt_enable) && (state.WB.Wrt_reg_addr == state.EX.Rs)) {
                state.EX.Read_data1 = state.WB.Wrt_data;
                cout<<"MEM-EX Rs Forwarding"<<endl;
            }
            if ((0 == state.WB.nop) && (1 == state.WB.wrt_enable) && (state.WB.Wrt_reg_addr == state.EX.Rt)) {
                if (((0 == state.EX.is_I_type) && (1 == state.EX.wrt_enable)) || (1 == state.EX.wrt_mem))   //addu, subu, sw
                {
                    state.EX.Read_data2 = state.WB.Wrt_data;
                    cout<<"MEM-EX Rt Forwarding"<<endl;                
                }
            }
            if ((0 == state.MEM.nop) && (0 == state.MEM.rd_mem) && (0 == state.MEM.wrt_mem) && (1 == state.MEM.wrt_enable) && (state.MEM.Wrt_reg_addr == state.EX.Rs)) {               
                state.EX.Read_data1 = state.MEM.ALUresult;
                cout<<"EX-EX Rs Forwarding"<<endl;
            }
            
            if ((0 == state.MEM.nop) && (0 == state.MEM.rd_mem) && (0 == state.MEM.wrt_mem) && (1 == state.MEM.wrt_enable) && (state.MEM.Wrt_reg_addr == state.EX.Rt)) {
                if ((0 == state.EX.is_I_type) && (1 == state.EX.wrt_enable)) {  // || (1 == state.EX.wrt_mem))   //addu, subu, for sw, we choose MEM-MEM but EX-EX
                    state.EX.Read_data2 = state.MEM.ALUresult;
                    cout<<"EX-EX Rt Forwarding"<<endl; 
                }
            }            
            
            if (0 == state.EX.is_I_type) {
                if (1 == state.EX.wrt_enable) {
                    if (state.EX.alu_op == 1) {
                        nextState.MEM.ALUresult = state.EX.Read_data1.to_ulong() + state.EX.Read_data2.to_ulong();
                        //cout<<"addu subu ALUresult:\t"<<nextState.MEM.ALUresult<<endl;                
                    } else if (state.EX.alu_op == 0) {
                        nextState.MEM.ALUresult = state.EX.Read_data1.to_ulong() - state.EX.Read_data2.to_ulong();
                        //cout<<"addu subu ALUresult:\t"<<nextState.MEM.ALUresult<<endl;                
                    }
                } else {
                    nextState.MEM.ALUresult = 0; //case of branch
                }
            } else if (1 == state.EX.is_I_type) {
                nextState.MEM.ALUresult = state.EX.Read_data1.to_ulong() + sign_extend(state.EX.Imm).to_ulong();
            }
            nextState.MEM.Store_data = state.EX.Read_data2;
            nextState.MEM.Rs = state.EX.Rs;
            nextState.MEM.Rt = state.EX.Rt;            
            nextState.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;              
            nextState.MEM.wrt_enable = state.EX.wrt_enable;           
            nextState.MEM.rd_mem = state.EX.rd_mem;
            nextState.MEM.wrt_mem = state.EX.wrt_mem;
            //cout<<"Rs_data:\t"<<state.EX.Read_data1.to_ulong()<<endl;
            //cout<<"Rt_data:\t"<<state.EX.Read_data2.to_ulong()<<endl;
            //cout<<"alu_op:\t"<<state.EX.alu_op<<endl;                            
            //cout<<"addu subu ALUresult:\t"<<nextState.MEM.ALUresult<<endl;            
        	}
        	nextState.MEM.nop = state.EX.nop;        
        	nextState.MEM.INS = state.EX.INS;
			
			
			/* --------------------- EX stage --------------------- */
			if (0 == state.EX.nop) {                       
            if ((0 == state.WB.nop) && (1 == state.WB.wrt_enable) && (state.WB.Wrt_reg_addr == state.EX.Rs)) {
                state.EX.Read_data1 = state.WB.Wrt_data;
                cout<<"MEM-EX Rs Forwarding"<<endl;
            }
            if ((0 == state.WB.nop) && (1 == state.WB.wrt_enable) && (state.WB.Wrt_reg_addr == state.EX.Rt)) {
                if (((0 == state.EX.is_I_type) && (1 == state.EX.wrt_enable)) || (1 == state.EX.wrt_mem))   //addu, subu, sw
                {
                    state.EX.Read_data2 = state.WB.Wrt_data;
                    cout<<"MEM-EX Rt Forwarding"<<endl;                
                }
            }
            if ((0 == state.MEM.nop) && (0 == state.MEM.rd_mem) && (0 == state.MEM.wrt_mem) && (1 == state.MEM.wrt_enable) && (state.MEM.Wrt_reg_addr == state.EX.Rs)) {               
                state.EX.Read_data1 = state.MEM.ALUresult;
                cout<<"EX-EX Rs Forwarding"<<endl;
            }
            if ((0 == state.MEM.nop) && (0 == state.MEM.rd_mem) && (0 == state.MEM.wrt_mem) && (1 == state.MEM.wrt_enable) && (state.MEM.Wrt_reg_addr == state.EX.Rt)) {
                if ((0 == state.EX.is_I_type) && (1 == state.EX.wrt_enable)) {  // || (1 == state.EX.wrt_mem))   //addu, subu, for sw, we choose MEM-MEM but EX-EX
                    state.EX.Read_data2 = state.MEM.ALUresult;
                    cout<<"EX-EX Rt Forwarding"<<endl; 
                }
            }            
            
            if (0 == state.EX.is_I_type) {
                if (1 == state.EX.wrt_enable) {
                    if (state.EX.alu_op == 1) {
                        nextState.MEM.ALUresult = state.EX.Read_data1.to_ulong() + state.EX.Read_data2.to_ulong();
                        //cout<<"addu subu ALUresult:\t"<<nextState.MEM.ALUresult<<endl;                
                    }
                    else if (state.EX.alu_op == 0) {
                        nextState.MEM.ALUresult = state.EX.Read_data1.to_ulong() - state.EX.Read_data2.to_ulong();
                        //cout<<"addu subu ALUresult:\t"<<nextState.MEM.ALUresult<<endl;                
                    }
                } else {
                    nextState.MEM.ALUresult = 0; //case of branch
                }
            } else if (1 == state.EX.is_I_type) {
                nextState.MEM.ALUresult = state.EX.Read_data1.to_ulong() + sign_extend(state.EX.Imm).to_ulong();
            }
            nextState.MEM.Store_data = state.EX.Read_data2;
            nextState.MEM.Rs = state.EX.Rs;
            nextState.MEM.Rt = state.EX.Rt;            
            nextState.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;              
            nextState.MEM.wrt_enable = state.EX.wrt_enable;           
            nextState.MEM.rd_mem = state.EX.rd_mem;
            nextState.MEM.wrt_mem = state.EX.wrt_mem;
            //cout<<"Rs_data:\t"<<state.EX.Read_data1.to_ulong()<<endl;
            //cout<<"Rt_data:\t"<<state.EX.Read_data2.to_ulong()<<endl;
            //cout<<"alu_op:\t"<<state.EX.alu_op<<endl;                            
            //cout<<"addu subu ALUresult:\t"<<nextState.MEM.ALUresult<<endl;            
    		}
        	nextState.MEM.nop = state.EX.nop;        
        	nextState.MEM.INS = state.EX.INS;
			/* --------------------- ID stage --------------------- */
			if (0 == state.ID.nop) {
				Instr = state.ID.Instr;
				opcode = Instr.to_string().substr(0,6);		//decode instruction
				func = Instr.to_string().substr(26,6);

				Rs = bitset<5>(Instr.to_string().substr(6,5));
				nextState.EX.Rs = Rs;
				nextState.EX.Read_data1 = myRF.readRF(Rs);   
			
				Rt = bitset<5>(Instr.to_string().substr(11,5));
				nextState.EX.Rt = Rt;
				nextState.EX.Read_data2 = myRF.readRF(Rt);
				
				Imm = bitset<16>(Instr.to_string().substr(16,16)); 
				nextState.EX.Imm = Imm;
            
            	Rd = bitset<5>(Instr.to_string().substr(16,5));
        
            	//cout<<"Rs:\t"<<Rs<<endl;
            	//cout<<"Rt:\t"<<Rt<<endl;   
				if (opcode == "000000")	{//execution            
					nextState.EX.Wrt_reg_addr = Rd; 
					nextState.EX.is_I_type = 0;
					if (func == "100001") {//addu
					
						nextState.EX.INS = "addu";                    
						nextState.EX.alu_op = 1;     
					}
					else if (func == "100011")	{//subu
						nextState.EX.INS = "subu";                    
						nextState.EX.alu_op = 0;      
					}
					nextState.EX.wrt_enable = 1;
					nextState.EX.rd_mem = 0;
					nextState.EX.wrt_mem = 0;                 
				}            

				else if (opcode == "100011") {//Load Word R[rt] = M[R[rs]+SignExtImm]
					nextState.EX.INS = "lw";                
					nextState.EX.Wrt_reg_addr = Rt;
					nextState.EX.is_I_type = 1;               
					nextState.EX.alu_op = 1;
					nextState.EX.wrt_enable = 1;                
					nextState.EX.rd_mem = 1;
					nextState.EX.wrt_mem = 0;                      
				}

				else if (opcode == "101011") {//Store Word M[R[rs]+SignExtImm] = R[rt]
					nextState.EX.INS = "sw";       
					nextState.EX.Wrt_reg_addr = Rt;  //will not be used
					nextState.EX.is_I_type = 1;                
					nextState.EX.alu_op = 1;
					nextState.EX.wrt_enable = 0;                
					nextState.EX.rd_mem = 0;
					nextState.EX.wrt_mem = 1;                 
				}
            
				else if (opcode == "000100") {//Branch On Equal if(R[rs]==R[rt]) PC=PC+4+BrachAddr
					nextState.EX.INS = "beq";
					nextState.EX.Wrt_reg_addr = 0;
					nextState.EX.is_I_type = 1;
					nextState.EX.alu_op = 1;
					nextState.EX.wrt_enable = 0;
					nextState.EX.rd_mem = 0;
					nextState.EX.wrt_mem = 0; 
					
					if (myRF.readRF(Rs) != myRF.readRF(Rt)) {
						cout<<"Branch not taken"<<endl;
						nextState.EX.nop = 0;
						nextState.ID.nop = 1;
						
						nextState.IF.PC = state.IF.PC.to_ulong() + bitset<30>(sign_extend(Imm).to_string().substr(2,30)).to_ulong()*4;
						nextState.IF.nop = 0;
						
						printState(nextState, cycle);     
						state = nextState;
						cycle ++;
						
						continue;                                       
					} 
					
					cout<<"Branch taken"<<endl;
				}
            
				if ((0 == state.EX.nop) && (1 == state.EX.rd_mem)) {//stall, will not consider branch after lw
				
					if ((state.EX.Wrt_reg_addr == Rs) || ((state.EX.Wrt_reg_addr == Rt) && (0 == nextState.EX.is_I_type))) { //stall               
						nextState.EX.nop = 1;
						nextState.ID = state.ID;
						nextState.IF = state.IF;
						printState(nextState, cycle);
						state = nextState;
						cycle ++;
						cout<<"Stall"<<endl;
						continue;
					}  
				}
        	}
        nextState.EX.nop = state.ID.nop;
			
			
			/* --------------------- IF stage --------------------- */
			// conditional nop by cooldown int array for all register
			if (0 == state.IF.nop) {
            //cout<<"PC:\t"<<state.IF.PC<<endl;
            bitset<32> Instruction= ext_imem.readInstr(state.IF.PC);            
            //cout<<"Instruction:\t"<<Instruction<<endl;
            if (Instruction != 0xffffffff) {
                nextState.IF.PC = state.IF.PC.to_ulong() + 4;
                nextState.IF.nop = 0;                               
            } else {
                state.IF.nop = 1;
                nextState.IF.PC = state.IF.PC.to_ulong();                
                nextState.IF.nop = 1;
                cout<<"PC:\t"<<state.IF.PC<<endl;                
            }
            	nextState.ID.Instr = Instruction;            
        	}
        	nextState.ID.nop = state.IF.nop;
			//halted = true;//stub
			if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop)
				halted = true;
        
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
		        printstate<<"EX.is_I_type:\t"<<state.EX.is_I_type<<endl; 
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
		if (!SSCore.halted)
			SSCore.step();
		
		if (!FSCore.halted)
			FSCore.step();

		if (SSCore.halted && FSCore.halted)
			break;
    }
    
	// dump SS and FS data mem.
	dmem_ss.outputDataMem();
	dmem_fs.outputDataMem();

	return 0;
}