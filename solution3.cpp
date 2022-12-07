if ((0 == state.WB.nop) && (1 == state.WB.wrt_enable) && (state.WB.Wrt_reg_addr == state.EX.Rs)) {
                state.EX.Read_data1 = state.WB.Wrt_data;
                cout<<"\tMEM-EX Rs Forwarding"<<endl;
				}
				if ((0 == state.WB.nop) && (1 == state.WB.wrt_enable) && (state.WB.Wrt_reg_addr == state.EX.Rt)) {
					if (((0 == state.EX.is_I_type) && (1 == state.EX.wrt_enable)) || (1 == state.EX.wrt_mem))   //addu, subu, sw
					{
						state.EX.Read_data2 = state.WB.Wrt_data;
						cout<<"\tMEM-EX Rt Forwarding"<<endl;                
					}
				}
				if ((0 == state.MEM.nop) && (0 == state.MEM.rd_mem) && (0 == state.MEM.wrt_mem) && (1 == state.MEM.wrt_enable) && (state.MEM.Wrt_reg_addr == state.EX.Rs)) {               
					state.EX.Read_data1 = state.MEM.ALUresult;
					cout<<"\tEX-EX Rs Forwarding"<<endl;
				}
				
				if ((0 == state.MEM.nop) && (0 == state.MEM.rd_mem) && (0 == state.MEM.wrt_mem) && (1 == state.MEM.wrt_enable) && (state.MEM.Wrt_reg_addr == state.EX.Rt)) {
					if ((0 == state.EX.is_I_type) && (1 == state.EX.wrt_enable)) {  // || (1 == state.EX.wrt_mem))   //addu, subu, for sw, we choose MEM-MEM but EX-EX
						state.EX.Read_data2 = state.MEM.ALUresult;
						cout<<"\tEX-EX Rt Forwarding"<<endl; 
					}
				}            
				
				if (0 == state.EX.is_I_type) {
					if (1 == state.EX.wrt_enable) {
						if (state.EX.alu_op == 1) {
							nextState.MEM.ALUresult = state.EX.Read_data1.to_ulong() + state.EX.Read_data2.to_ulong();
							cout<<"\taddu subu ALUresult:\t"<<nextState.MEM.ALUresult<<endl;                
						} else if (state.EX.alu_op == 0) {
							nextState.MEM.ALUresult = state.EX.Read_data1.to_ulong() - state.EX.Read_data2.to_ulong();
							cout<<"\taddu subu ALUresult:\t"<<nextState.MEM.ALUresult<<endl;                
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