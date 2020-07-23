module spi(
    input clk,
    input SCK,
    input MOSI,
    output MISO,
    input SSEL,
    output data_ready,
	output [7:0] data_recv,
	input [7:0] data_send,
    output [2:0] bit_count_out
);

logic [2:0] SCKr; always @(posedge clk) SCKr <= {SCKr[1:0], SCK};
logic SCK_risingedge; 
assign SCK_risingedge = (SCKr[2:1] == 2'b01);
logic SCK_fallingedge;
assign SCK_fallingedge = (SCKr[2:1] == 2'b10);

logic [2:0] SSELr; always @(posedge clk) SSELr <= {SSELr[1:0], SSEL};
logic SSEL_active;
assign SSEL_active = ~SSELr[1];
logic SSEL_startmessage;
assign SSEL_startmessage = (SSELr[2:1] == 2'b10);

logic [1:0] MOSIr; always @(posedge clk) MOSIr <= {MOSIr[0], MOSI};
logic MOSI_data;
assign MOSI_data = MOSIr[1];

logic [2:0] bit_count;
assign bit_count_out = bit_count;

logic byte_recv;
logic [7:0] byte_data_recv;

logic [7:0] byte_data_sent;
assign MISO = byte_data_sent[7];

logic data_ready_out;
assign data_ready = data_ready_out;

/* Control receiving data */
always @(posedge clk) begin
    if(~SSEL_active) begin
        bit_count <= 3'b000;
    end else if(SCK_risingedge) begin
        bit_count <= bit_count + 3'b001;

        byte_data_recv <= {byte_data_recv[6:0], MOSI_data};
    end
end

always @(posedge clk) byte_recv <= SSEL_active && SCK_risingedge && (bit_count == 3'b111);

/* Latch byte we just read here */
always @(posedge clk) if(byte_recv) data_ready_out <= byte_data_recv[7];

/* Control sending data */
always @(posedge clk) begin
    if(SSEL_active) begin
        if(SCK_fallingedge) begin
            if(bit_count == 3'b000) begin
                byte_data_sent <= data_send;
            end else begin
                byte_data_sent <= {byte_data_sent[6:0], 1'b0}; 
            end
        end
    end
end

endmodule