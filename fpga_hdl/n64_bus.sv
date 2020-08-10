module n64_bus(
    input clk,
    inout [15:0] AD,
    input READ,
    input WRITE,
    input ALE_L,
    input ALE_H,
    input [15:0] data,
    output addr_ready
);

logic [2:0] READr; always @(posedge clk) READr <= {READr[1:0], READ};
logic READ_risingedge; 
assign READ_risingedge = (READr[2:1] == 2'b01);
logic READ_fallingedge;
assign READ_fallingedge = (READr[2:1] == 2'b10);

logic [2:0] ALE_Lr; always @(posedge clk) ALE_Lr <= {ALE_Lr[1:0], ALE_L};
logic ALE_L_risingedge; 
assign ALE_L_risingedge = (ALE_Lr[2:1] == 2'b01);
logic ALE_L_fallingedge;
assign ALE_L_fallingedge = (ALE_Lr[2:1] == 2'b10);


logic [2:0] ALE_Hr; always @(posedge clk) ALE_Hr <= {ALE_Hr[1:0], ALE_H};
logic ALE_H_risingedge; 
assign ALE_H_risingedge = (ALE_Hr[2:1] == 2'b01);
logic ALE_H_fallingedge;
assign ALE_H_fallingedge = (ALE_Hr[2:1] == 2'b10);

logic addr_ready_out;
assign addr_ready = addr_ready_out;

logic [31:0] addr;

assign AD = (READ == 0) ? data : 16'bZ;

enum {
    state_wait,
    state_read_high,
    state_read_low
} state;

always @(posedge clk) begin
    case(state)
    state_wait: begin
        if(ALE_H_risingedge) begin
            addr_ready_out <= 0;
            state <= state_read_high;
        end
        if(READ_risingedge) begin
            addr_ready_out <= 1;
            addr <= addr + 2;
        end else if(READ_fallingedge) begin
            addr_ready_out <= 0;
        end
    end
    state_read_high: begin
        if(ALE_H_fallingedge) begin
            addr[31:16] <= AD;
            state <= state_read_low;
        end
    end
    state_read_low: begin
        if(ALE_L_fallingedge) begin
            addr_ready_out <= 1;
            addr[15:0] <= AD;
            state <= state_wait;
        end
    end
    endcase
end

endmodule