if (0 == state.ID.nop) {
				Instr = state.ID.Instr;
				cout<<"The instrtion is " << Instr << endl;
				func = Instr.to_string().substr(0,7);		//decode instruction
				opcode = Instr.to_string().substr(25,7);
				cout<<"The opcode is " << opcode << endl;
				cout<<"The function is " << func << endl;

				Rs = bitset<5>(Instr.to_string().substr(6,5));
				nextState.EX.Rs = Rs;
				nextState.EX.Read_data1 = myRF.readRF(Rs);   
			
				Rt = bitset<5>(Instr.to_string().substr(11,5));
				nextState.EX.Rt = Rt;
				nextState.EX.Read_data2 = myRF.readRF(Rt);
				
				Imm = bitset<16>(Instr.to_string().substr(16,16)); 
				nextState.EX.Imm = Imm;
            
            	Rd = bitset<5>(Instr.to_string().substr(16,5));
        
            	cout<<"Rs:\t"<<Rs<<endl;
            	cout<<"Rt:\t"<<Rt<<endl;   
				if (opcode == "0000000")	{//execution            
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
            
				else if (opcode == "1100011") {
					//Branch On Equal if(R[rs]==R[rt]) PC=PC+4+BrachAddr
					//Branch On not Equal if(R[rs]!=R[rt]) PC=PC+4+BrachAddr
					string func3 = Instr.to_string().substr(12,3);
					if (func3 == "000") {
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
							
							return;                                       
						} 
						
						cout<<"Branch taken for BEQ"<<endl;

					} else if (func3 == "010") {
						nextState.EX.INS = "bne";
						cout<<"BNE FOUND <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"<<endl;
						nextState.EX.Wrt_reg_addr = 0;
						nextState.EX.is_I_type = 1;
						nextState.EX.alu_op = 1;
						nextState.EX.wrt_enable = 0;
						nextState.EX.rd_mem = 0;
						nextState.EX.wrt_mem = 0; 
						
						if (myRF.readRF(Rs) == myRF.readRF(Rt)) {
							cout<<"Branch not taken"<<endl;
							nextState.EX.nop = 0;
							nextState.ID.nop = 1;
							
							nextState.IF.PC = state.IF.PC.to_ulong() + bitset<30>(sign_extend(Imm).to_string().substr(2,30)).to_ulong()*4;
							nextState.IF.nop = 0;
							
							printState(nextState, cycle);     
							state = nextState;
							cycle ++;
							
							return;                                       
						} 
						
						cout<<"Branch taken for BNE"<<endl;
					}
					
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
						return;
					}  
				}
        	}